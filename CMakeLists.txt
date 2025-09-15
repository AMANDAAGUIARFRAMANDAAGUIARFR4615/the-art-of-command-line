cmake_minimum_required(VERSION 3.16)
project(RemotePro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.h
    mainwindow.cpp
    videoplayer.h
    videoplayer.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt::Multimedia Qt::MultimediaWidgets)
