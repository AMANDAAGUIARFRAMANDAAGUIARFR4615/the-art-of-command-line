#include "Logger.h"
#include "MainWindow.h"
#include "TcpServer.h"
#include "UdpTransport.h"
#include "UdpTransport.h"
#include "ToastWidget.h"
#include "NetworkUtils.h"

#include <QApplication>

void onClientConnected() {
    qDebugT() << "有新的客户端连接！";
}

void onDataReceived(const QJsonObject &jsonObject) {
    qDebugT() << "接收到数据：" << jsonObject;
}

void onClientDisconnected() {
    qDebugT() << "客户端断开连接！";
}

void onError(QAbstractSocket::SocketError socketError) {
    qCriticalT() << "发生错误：" << socketError;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TcpServer server(onClientConnected, onDataReceived, onClientDisconnected, onError, 12345);

    QString localIP = NetworkUtils::getLocalIP();
    qDebugT() << "本机内网IP:" << localIP;

    MainWindow w;

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

    int x = (screenGeometry.width() - w.width()) / 2;
    int y = (screenGeometry.height() - w.height()) / 2;
    w.move(x, y);

    // w.show();
    w.showMinimized();

    return a.exec();
}
