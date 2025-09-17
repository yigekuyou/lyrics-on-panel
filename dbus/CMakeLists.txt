cmake_minimum_required(VERSION 4.0 )

project(mpris VERSION 0.1 LANGUAGES CXX)
find_package(Qt6 COMPONENTS Quick DBus Qml REQUIRED)
ecm_add_qml_module(mpris
    URI io.github.lyric
    GENERATE_PLUGIN_SOURCE
    VERSION 1.0
    QML_TYPES mpris.h mpris.cpp)
target_sources(mpris
    PRIVATE
    mpris.h mpris.cpp
)
target_link_libraries(mpris
    PRIVATE Qt6::Quick Qt6::DBus Qt6::Qml)
generate_export_header(mpris)

target_include_directories(mpris PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} Qt6::DBus Qt6::Quick Qt6::Qml )
ecm_finalize_qml_module(mpris)
