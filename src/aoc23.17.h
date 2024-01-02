#pragma once

#include <algorithm>
#include <chrono>
#include <format>
#include <iostream>
#include <map>
#include <string>
#include <vector>

void hello(std::string const &name);

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

enum class direction {
    NORTH,
    SOUTH,
    EAST,
    WEST
};


struct heat_loss_algorithm {
    explicit heat_loss_algorithm(city_map map)
        : map(std::move(map)) {}

    using position = city_map::position;
    city_map map;

    static constexpr position initial_position{0, 0};
    static constexpr auto maximal_heat_loss = std::numeric_limits<unsigned>::max();

    struct step {
        position start_pos;
        direction dir;
        bool direction_change;
    };
    std::vector<step> steps;
};

struct heat_loss_algorithm_brute_force : heat_loss_algorithm {
    using heat_loss_algorithm::heat_loss_algorithm;

    unsigned minimal_heat_loss = maximal_heat_loss;
    unsigned current_heat_loss = 0;
    position current_position = initial_position;

    [[nodiscard]] bool is_direction_change(direction dir) const {
        return steps.empty() || dir != steps.back().dir;
    }

    [[nodiscard]] auto next_position(direction dir) const {
        auto next = current_position;
        switch (dir) {
            case direction::NORTH:
                --next.y;
                break;
            case direction::SOUTH:
                ++next.y;
                break;
            case direction::EAST:
                ++next.x;
                break;
            case direction::WEST:
                --next.x;
                break;
        }
        return next;
    }

    void move(direction dir) {
        steps.emplace_back(current_position, dir, is_direction_change(dir));
        current_position = next_position(dir);
        current_heat_loss += map.heat_loss(current_position);
    }

    void undo() {
        current_heat_loss -= map.heat_loss(current_position);
        current_position = steps.empty() ? initial_position : steps.back().start_pos;
        steps.pop_back();
    }

    [[nodiscard]] bool arrived() const {
        return current_position.x == map.width() - 1 && current_position.y == map.height() - 1;
    }

    [[nodiscard]] bool last_three_steps_in_same_direction(direction dir) const {
        if (steps.size() < 3) return false;
        return std::all_of(steps.end() - 3, steps.end(), [dir](const auto &step) { return dir == step.dir; });
    }

    [[nodiscard]] bool last_step_opposite_direction(direction dir) const {
        if (steps.empty()) return false;
        switch (dir) {
            case direction::NORTH:
                return steps.back().dir == direction::SOUTH;
            case direction::SOUTH:
                return steps.back().dir == direction::NORTH;
            case direction::EAST:
                return steps.back().dir == direction::WEST;
            case direction::WEST:
                return steps.back().dir == direction::EAST;
        }
        return false;
    }

    [[nodiscard]] bool been_here_before(direction dir) const {
        return std::find_if(steps.begin(), steps.end(), [&](const auto &step) {
            return step.start_pos == next_position(dir);
        }) != steps.end();
//        if (is_direction_change(dir)) {
//            auto previous_direction_change_here = std::find_if(steps.begin(), steps.end(), [&](const auto &step) {
//                return step.direction_change && step.pos == next_position(dir) && step.dir == dir;
//            });
//            if (previous_direction_change_here != steps.end()) return true;
//        }
//        return false;
    }

    [[nodiscard]] bool can_move(direction dir) const {
//        if (steps.size() >= map.width() * map.height() * 20) {
//            return false;
//        }

        if (last_step_opposite_direction(dir)) return false;
        if (last_three_steps_in_same_direction(dir)) return false;


        const auto next = next_position(dir);
        constexpr auto underflow = std::numeric_limits<decltype(next.y)>::max();
        if (next.y == underflow) return false;
        if (next.y >= map.height()) return false;
        if (next.x == underflow) return false;
        if (next.x >= map.width()) return false;

        if (been_here_before(dir)) return false;

        if (current_heat_loss + map.heat_loss(next) >= minimal_heat_loss) {
            return false;
        }

        return true;
    }


    void next_step() {
        for (auto dir: {direction::NORTH, direction::SOUTH, direction::EAST, direction::WEST}) {
            if (!can_move(dir)) {
                continue;
            }

            move(dir);

            if (current_heat_loss < minimal_heat_loss && !arrived()) {
                next_step();
            }

            if (arrived()) {
                minimal_heat_loss = std::min(current_heat_loss, minimal_heat_loss);
            }

            undo();
        }
    }
};


inline unsigned minimal_heat_loss(const city_map &map) {
    const auto start_time = std::chrono::steady_clock::now();
    heat_loss_algorithm_brute_force algorithm{map};
    algorithm.next_step();
    std::cerr << std::format("run time: {}", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time));
    return algorithm.minimal_heat_loss;
}
