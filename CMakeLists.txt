cmake_minimum_required(VERSION 3.16)
project(WriteHand VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg PrintSupport)

qt_standard_project_setup()

qt_add_executable(WriteHand
    main.cpp
    MainWindow.cpp
    MainWindow.h
    EditorWidget.cpp
    EditorWidget.h
    FileTreeWidget.cpp
    FileTreeWidget.h
    WelcomeWidget.cpp
    WelcomeWidget.h
    ThemeManager.cpp
    ThemeManager.h
    ColumnView.cpp
    ColumnView.h
)

target_link_libraries(WriteHand PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::PrintSupport
)

set_target_properties(WriteHand PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
) 