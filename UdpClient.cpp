#include "UdpClient.h"
#include <QTextStream>
#include <QDebug>
#include <QHostAddress>
#include <QJsonDocument>

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

void UdpClient::sendData(const QJsonObject &jsonObject, const QString &ip, quint16 port)
{
    if (socket->state() != QAbstractSocket::BoundState) {
        qDebug() << "UDP 套接字未绑定，无法发送数据！";
        return;
    }

    // 8字节识别码
    quint64 identifier = 0xc6e8f3de9a654d6b;

    // 将 JSON 数据转换为 QByteArray
    QJsonDocument doc(jsonObject);
    QByteArray jsonData = doc.toJson();

    // 获取 JSON 数据的长度
    quint32 jsonDataLength = jsonData.size();

    // 创建数据包，并按顺序添加识别码、长度、JSON 数据
    QByteArray dataToSend;

    // 添加识别码
    dataToSend.append(reinterpret_cast<const char*>(&identifier), sizeof(identifier));

    // 添加 JSON 数据的长度
    dataToSend.append(reinterpret_cast<const char*>(&jsonDataLength), sizeof(jsonDataLength));

    // 添加 JSON 数据
    dataToSend.append(jsonData);

    // 发送数据
    socket->writeDatagram(dataToSend, QHostAddress(ip), port);
    socket->flush();
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
