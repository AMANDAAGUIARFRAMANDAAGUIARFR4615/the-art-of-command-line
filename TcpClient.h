#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <functional>
#include <QByteArray>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    TcpClient(const std::function<void()> &onConnected,
              const std::function<void(const QByteArray&)> &onDataReceived,
              const std::function<void()> &onDisconnected,
              const std::function<void(QAbstractSocket::SocketError)> &onError);

    void sendData(const QString &data);

private:
    QTcpSocket *socket;
    std::function<void()> onConnectedCallback;
    std::function<void(const QByteArray&)> onDataReceivedCallback;
    std::function<void()> onDisconnectedCallback;
    std::function<void(QAbstractSocket::SocketError)> onErrorCallback;
};

#endif // TCPCLIENT_H
