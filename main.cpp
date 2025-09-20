#include "Logger.h"
#include "MainWindow.h"
#include "TcpServer.h"
#include "UdpTransport.h"
#include "UdpTransport.h"
#include "ToastWidget.h"
#include "NetworkUtils.h"

#include <QApplication>

void onClientConnected(QTcpSocket* socket) {
    // qDebugT() << "有新的客户端连接！";
}

void onDataReceived(QTcpSocket* socket, const QJsonObject &jsonObject) {
    qDebugT() << "接收到数据：" << jsonObject;
}

void onClientDisconnected(QTcpSocket* socket) {
    // qDebugT() << "客户端断开连接！";
}

void onError(QTcpSocket* socket, QAbstractSocket::SocketError socketError) {
    qCriticalT() << "发生错误：" << socketError;
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    TcpServer server(onClientConnected, onDataReceived, onClientDisconnected, onError, 12345);

    QString localIP = NetworkUtils::getLocalIP();
    qDebugT() << "本机内网IP:" << localIP;

    MainWindow mainWindow;

    UdpTransport udpTransport(
        [](const QJsonObject &jsonObject) {
            qDebugT() << "Received Data:" << jsonObject;
        },
        [](QAbstractSocket::SocketError error) {
            qCriticalT() << "Error:" << error;
        }
    );

    QList<QHostAddress> subnetIPs = NetworkUtils::getSubnetIPs(localIP);
    for (const QHostAddress &ip : subnetIPs) {
        // qDebugT() << "同子网IP: " << ip.toString();
        udpTransport.sendData(QJsonObject{{"ip", localIP}, {"port", server.serverPort()}}, ip, 32838);
    }

    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();

    int x = (screenGeometry.width() - mainWindow.width()) / 2;
    int y = (screenGeometry.height() - mainWindow.height()) / 2;
    mainWindow.move(x, y);

    // mainWindow.show();
    mainWindow.showMinimized();

    return application.exec();
}
