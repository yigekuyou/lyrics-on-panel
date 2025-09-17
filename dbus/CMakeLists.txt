cmake_minimum_required(VERSION 4.0 )

project(mpris VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 COMPONENTS Quick DBus Qml REQUIRED)
add_library(mpris SHARED)
qt_add_qml_module(mpris
    URI "io.github.lyric"
    VERSION 1.0
    SOURCES mpris.cpp mpris.h
)

target_compile_definitions(mpris
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(mpris
    PRIVATE Qt6::Quick Qt6::DBus Qt6::Qml)

target_include_directories(mpris PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} Qt6::DBus Qt6::Quick Qt6::Qml )

install(TARGETS mpris DESTINATION ${KDE_INSTALL_QMLDIR}/io/github/lyric)


install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/io/github/lyric)
