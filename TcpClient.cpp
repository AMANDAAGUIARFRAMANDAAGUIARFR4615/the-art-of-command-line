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
    socket->connectToHost("127.0.0.1", 3000);

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

void TcpClient::sendData(const QString &data)
{
    QTextStream stream(socket);
    stream << data << Qt::endl;
    socket->flush();
}
