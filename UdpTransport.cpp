#include "UdpTransport.h"
#include "Logger.h"
#include <QTextStream>
#include <QDebug>
#include <QHostAddress>
#include <QJsonDocument>

UdpTransport::UdpTransport(const std::function<void(const QJsonObject &jsonObject)> &onDataReceived,
                     const std::function<void(QAbstractSocket::SocketError)> &onError,
                     quint16 listenPort)
    : onDataReceivedCallback(onDataReceived),
      onErrorCallback(onError),
      listenPort(listenPort)
{
    socket = new QUdpSocket(this);

    if (!socket->bind(QHostAddress::AnyIPv4, listenPort)) {
        qCriticalT() << "无法绑定端口";
        return;
    }

    connect(socket, &QUdpSocket::readyRead, this,  [this]() {
        QByteArray receivedData;
        while (socket->hasPendingDatagrams()) {
            receivedData.resize(int(socket->pendingDatagramSize()));
            socket->readDatagram(receivedData.data(), receivedData.size());

            // 将接收到的数据缓存到缓冲区
            buffer.append(receivedData);

            // 尝试处理缓冲区中的数据包
            processBufferedData();
        }
    });

    connect(socket, &QUdpSocket::errorOccurred, this,  [this](QAbstractSocket::SocketError error) {
        if (onErrorCallback) {
            onErrorCallback(error);
        }
    });
}

void UdpTransport::sendData(const QJsonObject &jsonObject, const QHostAddress &host, quint16 port)
{
    if (socket->state() != QAbstractSocket::BoundState) {
        qCriticalT() << "UDP 套接字未绑定，无法发送数据！";
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
    socket->writeDatagram(dataToSend, host, port);
    socket->flush();
}

void UdpTransport::processBufferedData()
{
    while (buffer.size() >= sizeof(quint64) + sizeof(quint32)) {
        // 解析识别码
        quint64 identifier = *reinterpret_cast<quint64*>(buffer.data());
        if (identifier != 0xb7c2e0f542a39a3e) {
            qCriticalT() << "识别码不匹配，丢弃数据";
            buffer.clear(); // 清空缓冲区
            return;
        }

        // 解析 JSON 数据长度
        quint32 jsonDataLength = *reinterpret_cast<quint32*>(buffer.data() + sizeof(quint64));

        // 检查缓冲区是否包含完整的数据包
        if (buffer.size() < sizeof(quint64) + sizeof(quint32) + jsonDataLength) {
            // 数据不完整，等待更多数据
            return;
        }

        // 提取 JSON 数据
        QByteArray jsonData = buffer.mid(sizeof(quint64) + sizeof(quint32), jsonDataLength);
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (!doc.isNull()) {
            if (onDataReceivedCallback) {
                onDataReceivedCallback(doc.object());
            }
        } else {
            qCriticalT() << "JSON 解析失败，丢弃数据";
        }

        // 移除已处理的数据包
        buffer.remove(0, sizeof(quint64) + sizeof(quint32) + jsonDataLength);
    }
}
