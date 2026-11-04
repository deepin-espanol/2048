cmake_minimum_required(VERSION 3.16)
project(2048 VERSION 0.3.2 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

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
    src/soundmanager.cpp
    src/soundmanager.h
    src/settingsdialog.cpp
    src/settingsdialog.h
)

target_link_libraries(2048 PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Dtk6::Widget
)

install(TARGETS 2048 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES data/2048-deepin.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES src/icons/2048.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME 2048-deepin.svg)
