#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <functional>
#include <QByteArray>

class UdpClient : public QObject
{
    Q_OBJECT

public:
    UdpClient(quint16 listenPort,
              const std::function<void()> &onDataReceived,
              const std::function<void()> &onError);

    void sendData(const QString &data, const QString &ip, quint16 port);

private slots:
    void onDataReceivedInternal();
    void onErrorInternal(QAbstractSocket::SocketError error);

private:
    QUdpSocket *socket;
    quint16 listenPort;
    std::function<void()> onDataReceivedCallback;
    std::function<void()> onErrorCallback;
};

#endif // UDPCLIENT_H
