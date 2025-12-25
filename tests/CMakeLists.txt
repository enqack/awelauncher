find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(test_fuzzy
    test_fuzzy.cpp
    ../src/App/utils/FuzzyMatcher.cpp
)

target_include_directories(test_fuzzy PRIVATE ../src)
target_link_libraries(test_fuzzy PRIVATE Qt6::Test)

add_test(NAME test_fuzzy COMMAND test_fuzzy)

add_executable(test_theme
    test_theme.cpp
    ../src/App/utils/Theme.cpp
    ../src/App/utils/ThemeScanner.cpp
    ../src/App/utils/Config.cpp
    ../src/App/utils/FuzzyMatcher.cpp 
    # FuzzyMatcher might not be needed but just in case of indirect deps, 
    # but strictly checking imports: Theme -> Config -> yaml-cpp
)

target_include_directories(test_theme PRIVATE ../src)
target_link_libraries(test_theme PRIVATE Qt6::Test Qt6::Gui Qt6::Quick yaml-cpp)
# Note: Theme.cpp uses QColor which needs Gui. 

add_test(NAME test_theme COMMAND test_theme)
