cmake_minimum_required(VERSION 3.16)

project(mpris VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.2 COMPONENTS Quick DBus Qml REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
add_library(mpris SHARED)
qt_add_qml_module(mpris
    URI "com.mpris.lyric"
    VERSION 1.0
    SOURCES mpris.cpp mpris.h
)
install(SCRIPT ${deploy_script})
target_compile_definitions(mpris
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(mpris
    PRIVATE Qt6::Quick Qt6::DBus Qt6::Qml)

target_include_directories(mpris PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} Qt6::DBus)
install(TARGETS mpris
    DESTINATION ${QT_INSTALL_QML}/com/mpris/lyric
)
