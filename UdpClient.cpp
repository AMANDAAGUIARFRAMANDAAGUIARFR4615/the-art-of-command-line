#include "UdpClient.h"
#include <QTextStream>
#include <QDebug>
#include <QHostAddress>

UdpClient::UdpClient(const std::function<void()> &onDataReceived,
                     const std::function<void()> &onError)
    : onDataReceivedCallback(onDataReceived),
      onErrorCallback(onError)
{
    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::Any, 0);  // 绑定到一个本地端口

    connect(socket, &QUdpSocket::readyRead, this, &UdpClient::onDataReceivedInternal);
    connect(socket, &QUdpSocket::errorOccurred, this, &UdpClient::onErrorInternal);
}

void UdpClient::sendData(const QString &data)
{
    QByteArray datagram = data.toUtf8();
    socket->writeDatagram(datagram, QHostAddress("127.0.0.1"), 3000);  // 发送到服务器
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
