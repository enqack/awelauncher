cmake_minimum_required(VERSION 3.16)

# Suppress developer warnings (like Qt policy warnings)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON)

file(READ "VERSION" APP_VERSION)
string(STRIP "${APP_VERSION}" APP_VERSION)

project(awelauncher
    VERSION ${APP_VERSION}
    LANGUAGES C CXX
)

add_compile_definitions(APP_VERSION="${APP_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Gui WaylandClient)
find_package(LayerShellQt REQUIRED)
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
    src/App/utils/ThemeScanner.cpp
    src/App/utils/ThemeScanner.h
    src/App/utils/Config.cpp
    src/App/utils/Config.h
    src/App/providers/IconProvider.cpp
    src/App/providers/IconProvider.h
    src/App/providers/DesktopFileLoader.cpp
    src/App/providers/DesktopFileLoader.h
    src/App/providers/WindowProvider.cpp
    src/App/providers/WindowProvider.h
    src/App/utils/FuzzyMatcher.cpp
    src/App/utils/FuzzyMatcher.h
    src/App/utils/MRUTracker.cpp
    src/App/utils/MRUTracker.h
    src/App/providers/StdinProvider.cpp
    src/App/providers/StdinProvider.h
)

target_include_directories(awelaunch PRIVATE src)

# Find wayland packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(WAYLAND_SCANNER REQUIRED wayland-scanner)

# Get wayland-scanner executable
pkg_get_variable(WAYLAND_SCANNER_EXEC wayland-scanner wayland_scanner)

# Generate protocol bindings for wlr-foreign-toplevel
set(PROTOCOL_XML "${CMAKE_SOURCE_DIR}/src/wayland-protocols/wlr-foreign-toplevel-management-unstable-v1.xml")
set(PROTOCOL_CLIENT_HEADER "${CMAKE_BINARY_DIR}/wlr-foreign-toplevel-client-protocol.h")
set(PROTOCOL_CODE "${CMAKE_BINARY_DIR}/wlr-foreign-toplevel-protocol.c")

add_custom_command(
    OUTPUT "${PROTOCOL_CLIENT_HEADER}"
    COMMAND ${WAYLAND_SCANNER_EXEC} client-header "${PROTOCOL_XML}" "${PROTOCOL_CLIENT_HEADER}"
    DEPENDS "${PROTOCOL_XML}"
    VERBATIM
)

add_custom_command(
    OUTPUT "${PROTOCOL_CODE}"
    COMMAND ${WAYLAND_SCANNER_EXEC} private-code "${PROTOCOL_XML}" "${PROTOCOL_CODE}"
    DEPENDS "${PROTOCOL_XML}"
    VERBATIM
)

# Add generated protocol files to build
add_library(wlr-foreign-toplevel-protocol STATIC
    ${PROTOCOL_CLIENT_HEADER}
    ${PROTOCOL_CODE}
)

# Ensure the .c file is compiled as C, not C++
set_source_files_properties(${PROTOCOL_CODE} PROPERTIES LANGUAGE C)

target_include_directories(wlr-foreign-toplevel-protocol PUBLIC
    ${CMAKE_BINARY_DIR}
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
)

target_link_libraries(wlr-foreign-toplevel-protocol PUBLIC
    ${WAYLAND_CLIENT_LIBRARIES}
)

target_include_directories(awelaunch PRIVATE 
    ${CMAKE_BINARY_DIR}
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
)

# Ensure protocol library is built before awelaunch
add_dependencies(awelaunch wlr-foreign-toplevel-protocol)

target_link_libraries(awelaunch
    PRIVATE
    Qt6::Quick
    Qt6::Gui
    Qt6::WaylandClient
    LayerShellQt::Interface
    yaml-cpp
    wlr-foreign-toplevel-protocol
    ${WAYLAND_CLIENT_LIBRARIES}
)

qt_add_qml_module(awelaunch
    URI awelauncher
    VERSION 1.0
    QML_FILES
        src/qml/LauncherRoot.qml
        src/qml/ResultRow.qml
        src/qml/SearchBar.qml
        src/qml/Footer.qml
        src/qml/HelpOverlay.qml
    RESOURCES
        examples/config/config.yaml
        examples/config/themes/default.yaml
        examples/config/themes/catppuccin.yaml
)

install(TARGETS awelaunch DESTINATION bin)

# Testing
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
