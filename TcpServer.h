#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QJsonObject>
#include <QMap>

class TcpServer : public QObject
{
    Q_OBJECT

public:
    TcpServer(const std::function<void()> &onClientConnected,
              const std::function<void(const QJsonObject&)> &onDataReceived,
              const std::function<void()> &onClientDisconnected,
              const std::function<void(QAbstractSocket::SocketError)> &onError,
              quint16 port = 0);

    quint16 getPort() const { return port; }

private slots:
    void onNewConnection();
    void onDataReceived();
    void onClientDisconnected();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpServer *server;
    QMap<QTcpSocket*, QByteArray> clientBuffers; // 保存每个客户端的缓冲区
    std::function<void()> onClientConnectedCallback;
    std::function<void(const QJsonObject&)> onDataReceivedCallback;
    std::function<void()> onClientDisconnectedCallback;
    std::function<void(QAbstractSocket::SocketError)> onErrorCallback;

    void processBufferedData(QTcpSocket* clientSocket);
    void sendData(QTcpSocket* clientSocket, const QJsonObject &jsonObject);
};

#endif // TCPSERVER_H
