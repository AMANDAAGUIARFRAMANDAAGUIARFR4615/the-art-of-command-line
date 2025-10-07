#pragma once

#include <QObject>
#include <QUdpSocket>
#include <functional>
#include <QByteArray>
#include <QJsonObject>

class UdpTransport : public QObject
{
    Q_OBJECT

public:
    UdpTransport(const std::function<void(const QJsonObject &jsonObject)> &onDataReceived,
              const std::function<void(QAbstractSocket::SocketError)> &onError = nullptr,
              quint16 listenPort = 0);

    void sendData(const QJsonObject &jsonObject, const QHostAddress &host, quint16 port, quint16 retryCount = 5);

private:
    std::function<void(const QJsonObject &jsonObject)> onDataReceivedCallback;
    std::function<void(QAbstractSocket::SocketError)> onErrorCallback;
    quint16 listenPort;

    QUdpSocket *socket;

    QByteArray buffer;
    void processBufferedData();
};
