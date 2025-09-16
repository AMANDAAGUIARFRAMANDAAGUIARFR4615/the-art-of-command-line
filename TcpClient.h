#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <functional>
#include <QByteArray>
#include <QJsonObject>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    TcpClient(const std::function<void()> &onConnected,
              const std::function<void(const QJsonObject&)> &onDataReceived,
              const std::function<void()> &onDisconnected,
              const std::function<void(QAbstractSocket::SocketError)> &onError);

    void connectToServer(const QString &ip, quint16 port);
    void sendData(const QJsonObject &jsonObject);

private:
    std::function<void()> onConnectedCallback;
    std::function<void(const QJsonObject&)> onDataReceivedCallback;
    std::function<void()> onDisconnectedCallback;
    std::function<void(QAbstractSocket::SocketError)> onErrorCallback;

    QTcpSocket *socket;

    QByteArray buffer;
    void processBufferedData();
};

#endif // TCPCLIENT_H
