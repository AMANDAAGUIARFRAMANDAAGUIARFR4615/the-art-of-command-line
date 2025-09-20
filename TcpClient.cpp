#include "TcpClient.h"
#include "Logger.h"
#include <QTextStream>
#include <QDebug>
#include <QJsonDocument>

TcpClient::TcpClient(const std::function<void()> &onConnected,
                     const std::function<void(const QJsonObject&)> &onDataReceived,
                     const std::function<void()> &onDisconnected,
                     const std::function<void(QAbstractSocket::SocketError)> &onError)
    : onConnectedCallback(onConnected),
      onDataReceivedCallback(onDataReceived),
      onDisconnectedCallback(onDisconnected),
      onErrorCallback(onError)
{
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, [this]() {
        QByteArray receivedData = socket->readAll();
        buffer.append(receivedData);
        processBufferedData();
    });

    connect(socket, &QTcpSocket::connected, [this]() {
        if (onConnectedCallback) {
            onConnectedCallback();
        }
    });

    connect(socket, &QTcpSocket::disconnected, [this]() {
        if (onDisconnectedCallback) {
            onDisconnectedCallback();
        }
    });

    connect(socket, &QTcpSocket::errorOccurred, [this](QAbstractSocket::SocketError error) {
        if (onErrorCallback) {
            onErrorCallback(error);
        }
    });
}

void TcpClient::connectToServer(const QString &ip, quint16 port)
{
    socket->connectToHost(ip, port);
    socket->waitForConnected();
}

void TcpClient::sendData(const QJsonObject &jsonObject)
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        qCriticalEx() << "未连接到服务器，无法发送数据！";
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
    socket->write(dataToSend);
    socket->flush();
}

void TcpClient::processBufferedData()
{
    while (buffer.size() >= sizeof(quint64) + sizeof(quint32)) {
        // 解析识别码
        quint64 identifier = *reinterpret_cast<quint64*>(buffer.data());
        if (identifier != 0xc6e8f3de9a654d6b) { // 检查识别码
            qCriticalEx() << "识别码不匹配，丢弃数据";
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
            qCriticalEx() << "JSON 解析失败，丢弃数据";
        }

        // 移除已处理的数据包
        buffer.remove(0, sizeof(quint64) + sizeof(quint32) + jsonDataLength);
    }
}
