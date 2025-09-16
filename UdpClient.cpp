#include "UdpClient.h"
#include <QTextStream>
#include <QDebug>
#include <QHostAddress>

UdpClient::UdpClient(quint16 listenPort,
                     const std::function<void()> &onDataReceived,
                     const std::function<void()> &onError)
    : listenPort(listenPort),
      onDataReceivedCallback(onDataReceived),
      onErrorCallback(onError)
{
    socket = new QUdpSocket(this);

    if (!socket->bind(QHostAddress::Any, listenPort)) {
        qDebug() << "无法绑定端口";
        return;
    }

    connect(socket, &QUdpSocket::readyRead, this, &UdpClient::onDataReceivedInternal);
    connect(socket, &QUdpSocket::errorOccurred, this, &UdpClient::onErrorInternal);
}

void UdpClient::sendData(const QString &data, const QString &ip, quint16 port)
{
    QByteArray datagram = data.toUtf8();
    socket->writeDatagram(datagram, QHostAddress(ip), port);
}

void UdpClient::onDataReceivedInternal()
{
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(socket->pendingDatagramSize()));
        socket->readDatagram(datagram.data(), datagram.size());

        if (onDataReceivedCallback) {
            onDataReceivedCallback();
        }
        qDebug() << "接收到数据: " << datagram;
    }
}

void UdpClient::onErrorInternal(QAbstractSocket::SocketError error)
{
    if (onErrorCallback) {
        onErrorCallback();
    }
    qDebug() << "发生错误: " << error;
}
