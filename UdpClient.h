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
    UdpClient(const std::function<void()> &onDataReceived,
              const std::function<void()> &onError);

    void sendData(const QString &data);

private slots:
    void onDataReceivedInternal();
    void onErrorInternal(QAbstractSocket::SocketError error);

private:
    QUdpSocket *socket;
    std::function<void()> onDataReceivedCallback;
    std::function<void()> onErrorCallback;
};

#endif // UDPCLIENT_H
