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

void TcpServer::processBufferedData(QTcpSocket* socket)
{
    auto &buffer = clientBuffers[socket];

    while (buffer.size() >= sizeof(quint64) + sizeof(quint32)) {
        quint64 identifier = *reinterpret_cast<quint64*>(buffer.data());
        if (identifier != 0xb7c2e0f542a39a3e) {
            qCriticalT() << "识别码不匹配，丢弃数据" << QString("0x%1").arg(identifier, 0, 16);
            buffer.clear(); // 清空缓冲区
            return;
        }

        quint32 jsonDataLength = *reinterpret_cast<quint32*>(buffer.data() + sizeof(quint64));

        if (buffer.size() < sizeof(quint64) + sizeof(quint32) + jsonDataLength) {
            qDebugT() << "数据不完整，等待更多数据";
            return;
        }

        QByteArray jsonData = buffer.mid(sizeof(quint64) + sizeof(quint32), jsonDataLength);
        // 移除已处理的数据包
        buffer.remove(0, sizeof(quint64) + sizeof(quint32) + jsonDataLength);
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        
        if (!doc.isNull()) {
            if (onDataReceivedCallback) {
                onDataReceivedCallback(doc.object());
            }
        } else {
            qCriticalT() << "JSON 解析失败，丢弃数据";
        }
    }
}

void TcpServer::sendData(QTcpSocket* socket, const QJsonObject &jsonObject)
{
    quint64 identifier = 0xc6e8f3de9a654d6b;

    QJsonDocument doc(jsonObject);
    QByteArray jsonData = doc.toJson();

    quint32 jsonDataLength = jsonData.size();

    QByteArray dataToSend;
    dataToSend.append(reinterpret_cast<const char*>(&identifier), sizeof(identifier));
    dataToSend.append(reinterpret_cast<const char*>(&jsonDataLength), sizeof(jsonDataLength));
    dataToSend.append(jsonData);

    socket->write(dataToSend);
    socket->flush();
}
