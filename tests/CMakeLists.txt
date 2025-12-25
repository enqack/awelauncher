find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(test_fuzzy
    test_fuzzy.cpp
    ../src/App/utils/FuzzyMatcher.cpp
)

target_include_directories(test_fuzzy PRIVATE ../src)
target_link_libraries(test_fuzzy PRIVATE Qt6::Test)

add_test(NAME test_fuzzy COMMAND test_fuzzy)
