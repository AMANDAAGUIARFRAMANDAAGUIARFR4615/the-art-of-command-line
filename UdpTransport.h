#ifndef UDPCLIENT_H
#define UDPCLIENT_H

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
              const std::function<void(QAbstractSocket::SocketError)> &onError,
              quint16 listenPort);

    void sendData(const QJsonObject &jsonObject, const QString &ip, quint16 port);

private:
    std::function<void(const QJsonObject &jsonObject)> onDataReceivedCallback;
    std::function<void(QAbstractSocket::SocketError)> onErrorCallback;
    quint16 listenPort;

    QUdpSocket *socket;

    QByteArray buffer;
    void processBufferedData();
};

#endif // UDPCLIENT_H
