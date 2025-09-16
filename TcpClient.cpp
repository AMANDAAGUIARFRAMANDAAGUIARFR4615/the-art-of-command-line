#include "TcpClient.h"
#include <QTextStream>
#include <QDebug>
#include <QJsonDocument>

TcpClient::TcpClient(const std::function<void()> &onConnected,
                     const std::function<void(const QByteArray&)> &onDataReceived,
                     const std::function<void()> &onDisconnected,
                     const std::function<void(QAbstractSocket::SocketError)> &onError)
    : onConnectedCallback(onConnected),
      onDataReceivedCallback(onDataReceived),
      onDisconnectedCallback(onDisconnected),
      onErrorCallback(onError)
{
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, [this]() {
        if (onDataReceivedCallback) {
            onDataReceivedCallback(socket->readAll());
        }
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
        qDebug() << "未连接到服务器，无法发送数据！";
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
