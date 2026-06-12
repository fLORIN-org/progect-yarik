cmake_minimum_required(VERSION 3.16)
project(prng_qt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(prng_qt
    main.cpp
    mainwindow.cpp
    chartwidget.h
)

target_link_libraries(prng_qt
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Для Windows: убираем консоль у финального .exe
if(WIN32)
    set_target_properties(prng_qt PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
