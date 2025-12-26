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

# Qt Quick Test (QML)
find_package(Qt6 REQUIRED COMPONENTS QuickTest)

add_executable(test_qml
    tst_main.cpp
    ../src/App/utils/Theme.cpp
    ../src/App/utils/ThemeScanner.cpp
    ../src/App/utils/Config.cpp
)

# We need to copy QML files to test execution directory or embed them.
# Embedding is easier for QuickTest.
qt_add_resources(test_qml "test_qml_res"
    PREFIX "/"
    FILES 
        ../src/qml/SearchBar.qml
        ../src/qml/Footer.qml
        qml/tst_SearchBar.qml
)

target_include_directories(test_qml PRIVATE ../src)
target_link_libraries(test_qml PRIVATE Qt6::Test Qt6::Quick Qt6::QuickTest yaml-cpp)

add_test(NAME test_qml COMMAND test_qml)
