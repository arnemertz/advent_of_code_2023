# All sources that also need to be tested in unit tests go into a static library
add_library(aoc_lib STATIC aoc23.17.cpp aoc23.17.h)
target_include_directories(aoc_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# The main program
add_executable(aoc_23 main.cpp)
target_link_libraries(aoc_23 PRIVATE aoc_lib)

