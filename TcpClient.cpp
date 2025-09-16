#include "TcpClient.h"
#include <QTextStream>
#include <QDebug>

TcpClient::TcpClient(const std::function<void()> &onConnected,
                     const std::function<void(const QByteArray&)> &onDataReceived,
                     const std::function<void()> &onDisconnected,
                     const std::function<void(QAbstractSocket::SocketError)> &onError)
    : onConnectedCallback(onConnected),
      onDataReceivedCallback(onDataReceived),
      onDisconnectedCallback(onDisconnected),
      onErrorCallback(onError)
{
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, [this]() {
        if (onDataReceivedCallback) {
            onDataReceivedCallback(socket->readAll());
        }
    });

    connect(socket, &QTcpSocket::connected, this, [this]() {
        if (onConnectedCallback) {
            onConnectedCallback();
        }
    });

    connect(socket, &QTcpSocket::disconnected, this, [this]() {
        if (onDisconnectedCallback) {
            onDisconnectedCallback();
        }
    });

    connect(socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
        if (onErrorCallback) {
            onErrorCallback(error);
        }
    });
}

void TcpClient::connectToServer(const QString &ip, quint16 port)
{
    socket->connectToHost(ip, port);
    socket->waitForConnected();
}

void TcpClient::sendData(const QJsonObject &jsonObject)
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "未连接到服务器，无法发送数据！";
        return;
    }

    QJsonDocument doc(jsonObject);
    QByteArray jsonData = doc.toJson();
    socket->write(jsonData);
    socket->flush();
}
