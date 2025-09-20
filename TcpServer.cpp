#include "TcpServer.h"
#include "Logger.h"
#include <QDebug>
#include <QJsonDocument>

TcpServer::TcpServer(const std::function<void()> &onClientConnected,
                     const std::function<void(const QJsonObject&)> &onDataReceived,
                     const std::function<void()> &onClientDisconnected,
                     const std::function<void(QAbstractSocket::SocketError)> &onError,
                     quint16 port)
    : onDataReceivedCallback(onDataReceived),
      onClientConnectedCallback(onClientConnected),
      onClientDisconnectedCallback(onClientDisconnected),
      onErrorCallback(onError)
{
    connect(this, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);

    if (!this->listen(QHostAddress::AnyIPv4, port)) {
        qCriticalT() << "无法启动服务器" << port;
        return;
    }

    qDebugT() << "服务器已启动,监听端口:" << this->serverPort();
}

void TcpServer::onNewConnection() {
    auto *clientSocket = this->nextPendingConnection();

    auto ip = clientSocket->peerAddress().toString();
    auto port = clientSocket->peerPort();

    qDebugT() << "新连接来自" << ip + ":" + QString::number(port);

    connect(clientSocket, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &TcpServer::onDisconnected);
    connect(clientSocket, &QTcpSocket::errorOccurred, this, &TcpServer::onErrorOccurred);

    clientBuffers[clientSocket] = QByteArray();
    onClientConnectedCallback();
}

void TcpServer::onReadyRead()
{
    auto *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        auto data = clientSocket->readAll();
        clientBuffers[clientSocket].append(data);
        processBufferedData(clientSocket);
    }
}

void TcpServer::onDisconnected()
{
    auto *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        clientBuffers.remove(clientSocket);
        clientSocket->deleteLater();
        onClientDisconnectedCallback();
    }
}

void TcpServer::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    qCriticalT() << "Socket error:" << socketError;
    onErrorCallback(socketError);
}

void TcpServer::processBufferedData(QTcpSocket* clientSocket)
{
    auto &buffer = clientBuffers[clientSocket];
    while (buffer.size() >= 8) { // 8字节为识别码大小
        quint64 identifier;
        QDataStream identifierStream(&buffer, QIODevice::ReadOnly);
        identifierStream >> identifier;

        if (buffer.size() < 8 + sizeof(quint32)) {
            break; // 如果数据不足，退出处理
        }

        quint32 dataSize;
        QDataStream sizeStream(&buffer, QIODevice::ReadOnly);
        sizeStream.skipRawData(8); // 跳过识别码
        sizeStream >> dataSize;

        if (buffer.size() < 8 + sizeof(quint32) + dataSize) {
            break; // 数据不完整，等待更多数据
        }

        QByteArray jsonData = buffer.mid(8 + sizeof(quint32), dataSize);
        buffer.remove(0, 8 + sizeof(quint32) + dataSize);

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isObject()) {
            QJsonObject jsonObject = doc.object();
            onDataReceivedCallback(jsonObject);
        }
    }
}

void TcpServer::sendData(QTcpSocket* clientSocket, const QJsonObject &jsonObject)
{
    QJsonDocument doc(jsonObject);
    QByteArray data = doc.toJson();

    QByteArray packet;
    quint64 identifier = 0xc6e8f3de9a654d6b;
    quint32 dataSize = data.size();

    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream << identifier << dataSize;
    packet.append(data);

    clientSocket->write(packet);
}
