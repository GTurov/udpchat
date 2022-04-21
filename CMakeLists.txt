cmake_minimum_required(VERSION 3.5)

project(udpchat LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

if(ANDROID)
  add_library(udpchat SHARED
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    networkprocessor.cpp
    networkprocessor.h
  )
else()
  add_executable(udpchat WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    networkprocessor.cpp
    networkprocessor.h
  )
endif()

target_link_libraries(udpchat PRIVATE Qt5::Widgets)
target_link_libraries(udpchat PRIVATE Qt5::Network)


