#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <format>
#include <iostream>
#include <map>
#include <queue>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

class city_map {
public:
    using row = std::vector<unsigned>;

    [[nodiscard]] auto width() const {
        return grid.empty() ? 0 : grid.front().size();
    }

    [[nodiscard]] auto height() const {
        return grid.size();
    }

    void add_row(row r) {
        if (!grid.empty() && r.size() != width()) {
            throw std::runtime_error(std::format("added row with wrong length: {} instead of {}", r.size(), width()));
        }
        grid.emplace_back(std::move(r));
    }

    struct position {
        std::size_t x;
        std::size_t y;

        auto operator<=>(const position &) const = default;
    };

    [[nodiscard]] unsigned heat_loss(const position &p) const {
        return grid.at(p.y).at(p.x);
    }


private:
    std::vector<row> grid;
};

enum class direction : int8_t {
    NORTH,
    SOUTH,
    EAST,
    WEST
};

constexpr direction opposite(direction dir) {
    switch (dir) {
        case direction::NORTH:
            return direction::SOUTH;
        case direction::SOUTH:
            return direction::NORTH;
        case direction::EAST:
            return direction::WEST;
        case direction::WEST:
            return direction::EAST;
    }
    return direction::NORTH;
}


struct heat_loss_algorithm {
    explicit heat_loss_algorithm(city_map map)
            : map(std::move(map)) {}

    using position = city_map::position;
    static constexpr auto maximal_heat_loss = std::numeric_limits<unsigned>::max();
    static constexpr position initial_position{0, 0};
    city_map map;
};

template<typename T, typename Weight>
struct prio_queue {
    struct element {
        Weight weight;
        T t;

        auto operator<=>(element const &other) const {
            return weight <=> other.weight;
        }
    };

    std::vector<element> elements;

    auto find_pos(Weight weight) {
        return std::lower_bound(elements.rbegin(), elements.rend(), weight,
                                [](const element &e, Weight w) { return e.weight < w; });
    }

    void add(T t, Weight weight) {
        elements.emplace(find_pos(weight).base(), weight, std::move(t));
    }

    [[nodiscard]] const element &top() const {
        return elements.back();
    }

    void reduceWeight(T const &t, Weight weight) {
        const auto new_pos = find_pos(weight);
        const auto old_pos = std::find_if(new_pos, elements.rend(), [&t](const element &e) { return e.t == t; });
        old_pos->weight = weight;

        std::rotate(new_pos, old_pos, old_pos + 1);
    }

    void pop() {
        elements.pop_back();
    }

    [[nodiscard]] bool empty() const {
        return elements.empty();
    }

};


struct heat_loss_algorithm_dijkstra : heat_loss_algorithm {
    struct step_history {
        direction dir;
        unsigned count;

        constexpr auto operator<=>(const step_history&) const = default;

        static constexpr unsigned max_count = 3;
    };

    struct node {
        position pos;
        step_history history;

        constexpr auto operator<=>(const node&) const = default;
    };

    prio_queue<node, unsigned> queue;
    std::map<node, unsigned> heat_loss;
    std::map<node, bool> visited;

    explicit heat_loss_algorithm_dijkstra(city_map map)
            : heat_loss_algorithm(std::move(map)) {
        prepare_nodes();
    }

    bool is_valid_history(const step_history history, const position pos) {
        if (pos == initial_position) {
            return history.dir == direction::EAST && history.count == 1;
        }
        if (history.dir == direction::NORTH) {
            return pos.y + history.count < map.height();
        }
        if (history.dir == direction::WEST) {
            return pos.x + history.count < map.width();
        }
        if (history.dir == direction::SOUTH) {
            return pos.y >= history.count;
        }
        return pos.x >= history.count;
    }

    void add_node(const node n, const unsigned hl) {
        queue.add(n, hl);
        heat_loss[n] = hl;
        visited[n] = false;
    }

    void prepare_nodes() {
        add_node(node{initial_position, step_history{direction::NORTH, 1}}, 0);
        for (unsigned x = 0; x < map.width(); ++x) {
            for (unsigned y = 0; y < map.height(); ++y) {
                const position pos{x, y};
                for (direction dir: {direction::NORTH, direction::SOUTH, direction::WEST, direction::EAST}) {
                    for (unsigned count: {1, 2, 3}) {
                        const auto history = step_history{dir, count};
                        if (is_valid_history(history, pos)) {
                            add_node(node{pos,history}, maximal_heat_loss);
                        }
                    }
                }
            }
        }
    }

    std::optional<position> neighbor_pos(position pos, direction dir) {
        switch (dir) {
            case direction::NORTH:
                return (pos.y == 0) ? std::nullopt : std::optional(position{pos.x, pos.y-1});
            case direction::SOUTH:
                return (pos.y == map.height()-1) ? std::nullopt : std::optional(position{pos.x, pos.y+1});
            case direction::EAST:
                return (pos.x == map.width()-1) ? std::nullopt : std::optional(position{pos.x+1, pos.y});
            case direction::WEST:
                return (pos.x == 0) ? std::nullopt : std::optional(position{pos.x-1, pos.y});
        }
        return std::nullopt;
    }

    auto& neighbors(const node& n)
    {
        thread_local std::vector<node> nodes;
        nodes.resize(0);

        const auto last_dir = n.history.dir;
        for (auto new_dir: {direction::NORTH, direction::SOUTH, direction::WEST, direction::EAST}) {
            if (new_dir == opposite(last_dir)) continue;

            step_history new_history{new_dir, 1};
            if (new_dir == last_dir) {
                if (n.history.count == 3) continue;
                else new_history.count += n.history.count;
            }

            const auto new_position = neighbor_pos(n.pos, new_dir);

            if (new_position.has_value()) {
                const auto neighbor = node{new_position.value(), new_history};
                if (!visited[neighbor]) {
                    nodes.emplace_back(new_position.value(), new_history);
                }
            }
        }
        return nodes;
    }

    void run_dijkstra() {
        const auto start_time = std::chrono::steady_clock::now();

        while (!queue.empty()) {
            const auto [current_weight, current_node] = queue.top();

            for (const auto& next_node : neighbors(current_node)) {
                const auto tentative_heat_loss = heat_loss[current_node] + map.heat_loss(next_node.pos);
                if (tentative_heat_loss < heat_loss[next_node]) {
                    heat_loss[next_node] = tentative_heat_loss;
                    queue.reduceWeight(next_node, tentative_heat_loss);
                }
            }

            queue.pop();
            visited[current_node] = true;
        }

        std::cerr << std::format("run time: {}", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start_time));
    }

    [[nodiscard]] auto get_minimal_heat_loss() const {
        position end = {map.width()-1, map.height()-1};

        unsigned minimal_heat_loss = maximal_heat_loss;
        for (direction dir : {direction::SOUTH, direction::EAST}) {
            for (unsigned count : {1,2,3}) {
                minimal_heat_loss = std::min(minimal_heat_loss, heat_loss.at(node{end, step_history{dir, count}}));
            }
        }
        return minimal_heat_loss;
    }
};

inline unsigned minimal_heat_loss(const city_map &map) {
    heat_loss_algorithm_dijkstra algorithm{map};
    algorithm.run_dijkstra();

    return algorithm.get_minimal_heat_loss();
}

