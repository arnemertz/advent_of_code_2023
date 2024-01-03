#include "aoc23.17.h"
#include <fstream>
#include <format>
#include <iostream>

int main() {
    city_map map;

    std::ifstream input("src/aoc23.17.input.txt");
    std::string line;
    std::vector<unsigned> row(line.size());
    while (std::getline(input, line)) {
        if (line.empty()) continue;
        row.resize(line.size());
        std::transform(line.begin(), line.end(), row.begin(),[](char c){
            return c-'0';
        });

        map.add_row(row);
    }
    std::cout << std::format("Width = {}, height = {}", map.width(), map.height());

    std::cout << std::format("minimal heat loss: {}", minimal_heat_loss_dijkstra(map));
}

