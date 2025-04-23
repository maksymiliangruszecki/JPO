cmake_minimum_required(VERSION 3.16)

project(test VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "x86_64")

INCLUDE(/Users/maksgruszecki/vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Charts Gui Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Charts Gui Core Widgets)


find_package(CURL REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        api_handle.h
        json.hpp
        datetimeaxiswidget.h
        datetimeaxiswidget.cpp
        contentwidget.h
        contentwidget.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(test
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(test SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(test
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(test PRIVATE JsonCpp::JsonCpp)
target_link_libraries(test PRIVATE Qt${QT_VERSION_MAJOR}::Widgets CURL::libcurl)

target_link_libraries(test PRIVATE
    Qt::Charts
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.test)
endif()
set_target_properties(test PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS test
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(test)
endif()
