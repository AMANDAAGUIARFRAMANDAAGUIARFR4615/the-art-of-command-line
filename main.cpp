#include "Logger.h"
#include "MainWindow.h"
#include "TcpServer.h"
#include "UdpTransport.h"
#include "UdpTransport.h"
#include "ToastWidget.h"
#include "NetworkUtils.h"

#include <QApplication>

void onClientConnected(QTcpSocket* socket) {
    // qDebugEx() << "有新的客户端连接！";
}

void onDataReceived(QTcpSocket* socket, const QJsonObject &jsonObject) {
    qDebugEx() << "接收到数据：" << jsonObject;

    if (jsonObject["event"].toString() == "ping")
        return;

    auto x = jsonObject["event"];

    qDebugEx() << x.toString();
}

void onClientDisconnected(QTcpSocket* socket) {
    // qDebugEx() << "客户端断开连接！";
}

void onError(QTcpSocket* socket, QAbstractSocket::SocketError socketError) {
    qCriticalEx() << "发生错误：" << socketError;
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    TcpServer server(onClientConnected, onDataReceived, onClientDisconnected, onError, 12345);

    QString localIP = NetworkUtils::getLocalIP();
    qDebugEx() << "本机内网IP:" << localIP;

    MainWindow mainWindow;

    UdpTransport udpTransport(
        [](const QJsonObject &jsonObject) {
            qDebugEx() << "Received Data:" << jsonObject;
        },
        [](QAbstractSocket::SocketError error) {
            qCriticalEx() << "Error:" << error;
        }
    );

    QList<QHostAddress> subnetIPs = NetworkUtils::getSubnetIPs(localIP);
    for (const QHostAddress &ip : subnetIPs) {
        // qDebugEx() << "同子网IP: " << ip.toString();
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
