cmake_minimum_required(VERSION 3.5)

project(mirror-selector VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus)
find_package(PkgConfig)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(mirror-selector
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        mirror-selector.qrc

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET mirror-selector APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(mirror-selector SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(mirror-selector
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(mirror-selector PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt::DBus)

add_executable(mirror-selector-commander
    commander.cpp
    mirror-selector.qrc

)
target_link_libraries(mirror-selector-commander PRIVATE Qt::Core PkgConfig::Systemd)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.mirror-selector)
endif()
set_target_properties(mirror-selector PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS mirror-selector
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(mirror-selector)
endif()


install(TARGETS mirror-selector RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS mirror-selector-commander RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES mirror-selector@.service DESTINATION lib/systemd/system/)
install(FILES mirror-selector.desktop DESTINATION share/applications/)
