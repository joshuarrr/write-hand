cmake_minimum_required(VERSION 3.16)
project(WritingApp VERSION 1.0)

# Enable parallel compilation
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add Qt installation path for macOS/Homebrew
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@6")

# Set installation directory to build directory
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")

# Set RPATH settings
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
set(CMAKE_BUILD_RPATH "@executable_path/../Frameworks")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus Svg)

# Set application properties
set(APP_NAME "WriteHand")
set(APP_BUNDLE_IDENTIFIER "com.yourcompany.writehand")

# Add source files
set(SOURCES
    main.cpp
    MainWindow.cpp
    EditorWidget.cpp
    FileTreeWidget.cpp
    WelcomeWidget.cpp
    ThemeManager.cpp
    ColumnView.cpp
)

# Add header files
set(HEADERS
    MainWindow.h
    EditorWidget.h
    FileTreeWidget.h
    WelcomeWidget.h
    ThemeManager.h
    ColumnView.h
)

# Create the application bundle
add_executable(${APP_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS})

# Enable precompiled headers
set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
target_precompile_headers(${APP_NAME} PRIVATE
    <QtWidgets>
    <QtCore>
    <QtGui>
    <string>
    <vector>
    <memory>
)

# Link Qt libraries
target_link_libraries(${APP_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Svg
)

# Set bundle properties
set_target_properties(${APP_NAME} PROPERTIES
    UNITY_BUILD ON
    UNITY_BUILD_BATCH_SIZE 4  # Adjust this number based on your needs
    MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_BUNDLE_NAME ${APP_NAME}
    MACOSX_BUNDLE_COPYRIGHT "Copyright © 2024. All rights reserved."
    MACOSX_BUNDLE_INFO_STRING "A minimalist writing app"
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "@executable_path/../Frameworks"
)

if(APPLE)
    # Find macdeployqt
    find_program(MACDEPLOYQT macdeployqt HINTS "/opt/homebrew/opt/qt@6/bin")
    if(NOT MACDEPLOYQT)
        message(FATAL_ERROR "macdeployqt not found")
    endif()

    # Install the application bundle
    install(TARGETS ${APP_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION bin
    )

    # Install icons
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/icons
        DESTINATION ${APP_NAME}.app/Contents/Resources
    )

    # Generate icons using our script
    install(CODE "
        execute_process(
            COMMAND ${CMAKE_SOURCE_DIR}/scripts/generate_icons.sh
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        execute_process(
            COMMAND cp -f ${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app/Contents/Resources/app.iconset/icon_512x512.png ${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app/Contents/Resources/${APP_NAME}.png
        )
        execute_process(
            COMMAND cp -f ${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app/Contents/Resources/app.iconset.icns ${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app/Contents/Resources/${APP_NAME}.icns
        )
    ")

    # Deploy Qt frameworks and plugins
    install(CODE "
        # First, deploy Qt frameworks and plugins
        execute_process(
            COMMAND ${MACDEPLOYQT} 
                \${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app 
                -verbose=2 
                -always-overwrite
                -no-strip
                -executable=\${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app/Contents/MacOS/${APP_NAME}
        )

        # Fix rpaths in plugins and frameworks
        execute_process(
            COMMAND find \${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app/Contents/PlugIns -name \"*.dylib\" -exec 
                install_name_tool -add_rpath \"@executable_path/../Frameworks\" {} \\;
        )
        execute_process(
            COMMAND find \${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app/Contents/Frameworks -name \"*\" -type f -exec 
                install_name_tool -add_rpath \"@executable_path/../Frameworks\" {} \\;
        )

        # Remove existing signatures
        execute_process(
            COMMAND codesign --remove-signature \${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app
        )

        # Sign the bundle with hardened runtime and entitlements
        execute_process(
            COMMAND codesign 
                --force 
                --deep 
                --sign - 
                --timestamp=none
                --options runtime 
                --entitlements ${CMAKE_SOURCE_DIR}/WriteHand.entitlements
                \${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app
        )

        # Verify the signing
        execute_process(
            COMMAND codesign --verify --deep --strict --verbose=2 \${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app
        )
    ")
endif()

# Copy icons to build directory
file(COPY ${CMAKE_SOURCE_DIR}/icons DESTINATION ${CMAKE_BINARY_DIR})

# Copy icons to app bundle on macOS
if(APPLE)
    add_custom_command(TARGET WriteHand POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_CONTENT_DIR:WriteHand>/Resources/icons"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/icons"
            "$<TARGET_BUNDLE_CONTENT_DIR:WriteHand>/Resources/icons"
    )
endif() 