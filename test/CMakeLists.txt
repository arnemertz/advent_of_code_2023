# The test program
add_executable(aoc_23_tests testmain.cpp)
target_link_libraries(aoc_23_tests PRIVATE aoc_lib catch)
target_compile_definitions(aoc_23_tests PRIVATE CATCH_CONFIG_CONSOLE_WIDTH=60)
