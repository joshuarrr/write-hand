cmake_minimum_required(VERSION 3.16)
project(WriteHand VERSION 0.1 LANGUAGES CXX)

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
    FontAwesome.cpp
    FontAwesome.h
    resources.qrc
)

target_link_libraries(WriteHand PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::PrintSupport
)

set_target_properties(WriteHand PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER com.joshuarichey.writehand
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.in
)

if(APPLE)
    set_target_properties(WriteHand PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/WriteHand.entitlements"
    )
endif() 