#include "aoc23.17.h"

#include "catch.hpp"

TEST_CASE("city_map") {
    city_map map;
    SECTION("checks width") {
        map.add_row({1, 2, 3, 4});
        CHECK_THROWS(map.add_row({1, 2, 3}));
    }
}

TEST_CASE("example case") {
    city_map map;
    map.add_row({2,4,1,3,4,3,2,3,1,1,3,2,3});
    map.add_row({3,2,1,5,4,5,3,5,3,5,6,2,3});
    map.add_row({3,2,5,5,2,4,5,6,5,4,2,5,4});
    map.add_row({3,4,4,6,5,8,5,8,4,5,4,5,2});
    map.add_row({4,5,4,6,6,5,7,8,6,7,5,3,6});
    map.add_row({1,4,3,8,5,9,8,7,9,8,4,5,4});
    map.add_row({4,4,5,7,8,7,6,9,8,7,7,6,6});
    map.add_row({3,6,3,7,8,7,7,9,7,9,6,5,3});
    map.add_row({4,6,5,4,9,6,7,9,8,6,8,8,7});
    map.add_row({4,5,6,4,6,7,9,9,8,6,4,5,3});
    map.add_row({1,2,2,4,6,8,6,8,6,5,5,6,3});
    map.add_row({2,5,4,6,5,4,8,8,8,7,7,3,5});
    map.add_row({4,3,2,2,6,7,4,6,5,5,5,3,3});
    CHECK(map.width() == 13);
    CHECK(map.height() == 13);

    REQUIRE(minimal_heat_loss(map) == 102);
}

