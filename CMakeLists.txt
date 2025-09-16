cmake_minimum_required(VERSION 3.16)
project(RemotePro LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

qt_standard_project_setup()

add_executable(${PROJECT_NAME}
    main.cpp
    MainWindow.h
    MainWindow.cpp
    VideoPlayer.h
    VideoPlayer.cpp
    TcpServer.h
    TcpServer.cpp
    TcpClient.h
    TcpClient.cpp
    UdpClient.h
    UdpClient.cpp
    ToastWidget.h
    NetworkUtils.h
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt::Multimedia Qt::MultimediaWidgets)
