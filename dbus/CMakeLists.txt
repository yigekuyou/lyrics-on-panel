cmake_minimum_required(VERSION 4.0 )

project(dbuslyric VERSION 0.1 LANGUAGES CXX)
find_package(Qt6 COMPONENTS Quick DBus Qml REQUIRED)
ecm_add_qml_module(dbuslyric
    URI io.github.lyric
    GENERATE_PLUGIN_SOURCE
    VERSION 1.0
    QML_TYPES mpris.h mpris.cpp)
target_sources(dbuslyric
    PRIVATE
    mpris.h mpris.cpp
)
target_link_libraries(dbuslyric
    PRIVATE Qt6::Quick Qt6::DBus Qt6::Qml)
generate_export_header(dbuslyric)

target_include_directories(dbuslyric PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} Qt6::DBus Qt6::Quick Qt6::Qml )
ecm_finalize_qml_module(dbuslyric)
add_subdirectory(test)
