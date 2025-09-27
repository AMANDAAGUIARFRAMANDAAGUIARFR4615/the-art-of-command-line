cmake_minimum_required(VERSION 3.16)
project(RemotePro LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Multimedia MultimediaWidgets)

qt_standard_project_setup()

qt_add_resources(RESOURCE_SOURCES icons.qrc)

add_executable(${PROJECT_NAME}
    main.cpp
    Logger.h
    LogWindow.h
    EventHub.h EventHub.cpp
    MainWindow.h MainWindow.cpp
    DeviceInfo.h DeviceControl.h
    RemoteDevice.h RemoteDevice.cpp
    TcpServer.h TcpServer.cpp
    # TcpClient.h TcpClient.cpp
    UdpTransport.h UdpTransport.cpp
    ToastWidget.h
    NetworkUtils.h
    ControlWindow.h ControlWindow.cpp
    RemoteFileExplorer.h RemoteFileExplorer.cpp FileTransfer.h
    ${RESOURCE_SOURCES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE

    # MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.qt6.remotepro"
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Network Qt::Multimedia Qt::MultimediaWidgets)

add_definitions(-DPROJECT_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\")
