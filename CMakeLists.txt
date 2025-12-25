cmake_minimum_required(VERSION 3.16)

project(awelauncher VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Gui WaylandClient)
find_package(yaml-cpp REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(awelaunch
    src/main.cpp
    src/App/controllers/LauncherController.cpp
    src/App/controllers/LauncherController.h
    src/App/models/LauncherModel.cpp
    src/App/models/LauncherModel.h
    src/App/utils/Theme.cpp
    src/App/utils/Theme.h
    src/App/utils/Config.cpp
    src/App/utils/Config.h
    src/App/providers/IconProvider.cpp
    src/App/providers/IconProvider.h
    src/App/providers/DesktopFileLoader.cpp
    src/App/providers/DesktopFileLoader.h
    src/App/utils/FuzzyMatcher.cpp
    src/App/utils/FuzzyMatcher.h
    src/App/utils/MRUTracker.cpp
    src/App/utils/MRUTracker.h
)

target_include_directories(awelaunch PRIVATE src)

target_link_libraries(awelaunch
    PRIVATE
    Qt6::Quick
    Qt6::Gui
    Qt6::WaylandClient
    yaml-cpp
)

qt_add_qml_module(awelaunch
    URI awelauncher
    VERSION 1.0
    QML_FILES
        src/qml/LauncherRoot.qml
        src/qml/ResultRow.qml
)

install(TARGETS awelaunch DESTINATION bin)
