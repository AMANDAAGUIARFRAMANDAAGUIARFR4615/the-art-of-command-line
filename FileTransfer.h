#pragma once

#include "Logger.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QFile>
#include <QDataStream>

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
        connect(socket, &QTcpSocket::readyRead, this, &FileTransfer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

        QFile file(path);

        if (!file.open(QIODevice::ReadOnly))
        {
            qCriticalEx() << "Failed to open file for sending.";
            return;
        }

        quint64 bytesSent = 0;
        while (!file.atEnd())
        {
            auto buffer = file.read(4096);
            qint64 written = socket->write(buffer);
            if (written == -1)
            {
                qCriticalEx() << "文件发送失败";
                break;
            }

            bytesSent += written;
            socket->waitForBytesWritten(3000);
            if (bytesSent >= file.size())
                break;
        }

        file.close();
        qInfoEx() << "文件发送成功";
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

private:
    QString path;
    quint64 size;
    QByteArray buffer;
};
