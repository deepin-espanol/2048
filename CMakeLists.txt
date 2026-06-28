cmake_minimum_required(VERSION 3.16)
project(2048 VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

find_package(Dtk6Widget REQUIRED)

add_executable(2048
    src/main.cpp
    src/GameBoard.cpp
    src/GameBoard.h
    src/MainWindow.cpp
    src/MainWindow.h
    src/widget.cpp
    src/widget.h
    src/widget.ui
    src/resources.qrc
)

target_link_libraries(2048 PRIVATE
    Qt6::Widgets
    Dtk6::Widget
)
