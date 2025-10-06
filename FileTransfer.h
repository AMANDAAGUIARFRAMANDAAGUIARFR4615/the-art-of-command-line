#pragma once

#include "Logger.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QFile>
#include <QDataStream>
#include <QtConcurrent>

class FileTransfer : public QTcpServer
{
    Q_OBJECT
public:
    FileTransfer(int type, const QString &path, quint64 size) : path(path), size(size)
    {
        connect(this, &QTcpServer::newConnection, this, &FileTransfer::onNewConnection);

        if (!listen(QHostAddress::Any, 0))
        {
            qWarning() << "Server failed to start";
        }
        else
        {
            qDebugEx() << "Server started, waiting for connections...";
        }
    }

protected:
    void onNewConnection()
    {
        auto socket = nextPendingConnection();

        close();
        qDebugEx() << "已接受第一个连接，服务器停止监听新连接。";

        connect(socket, &QTcpSocket::readyRead, this, &FileTransfer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
        connect(socket, &QTcpSocket::bytesWritten, this, &FileTransfer::onBytesWritten);  // 连接bytesWritten信号

        QtConcurrent::run([this, socket]() {
            QFile file(path);

            if (!file.open(QIODevice::ReadOnly))
            {
                qCriticalEx() << "Failed to open file for sending.";
                return;
            }

            while (!file.atEnd())
            {
                auto buffer = file.read(4096);
                QMetaObject::invokeMethod(this, [socket, buffer]() {
                    socket->write(buffer);
                });
            }

            file.close();
            qInfoEx() << "文件关闭";
        });
    }

    void onReadyRead()
    {
        QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
        auto data = socket->readAll();
        
        buffer.append(data);

        while (buffer.size() >= 8)
        {
            auto bytesSent = *reinterpret_cast<quint64 *>(buffer.data());
            buffer.remove(0, 8);

            if (size == bytesSent) {
                close();
                qDebugEx() << "发送完成断开连接";
            }
        }
    }

    void onBytesWritten(qint64 bytes)
    {
        QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
        if (bytes == socket->bytesToWrite())  // 数据完全写入
        {
            qDebugEx() << "文件已发送完成：" << bytes;
        }
    }

private:
    QString path;
    quint64 size;
    QByteArray buffer;
};
