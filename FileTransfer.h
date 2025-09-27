#pragma once

#include "Logger.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QTextStream>

class FileTransfer : public QTcpServer {
    Q_OBJECT
public:
    FileTransfer(int type, const QString& path, const QString& md5, qint64 size): path(path), md5(md5), size(size) {
        connect(this, &QTcpServer::newConnection, this, &FileTransfer::onNewConnection);

        if (!listen(QHostAddress::Any, 0)) {
            qWarning() << "Server failed to start";
        } else {
            qDebug() << "Server started, waiting for connections...";
        }
    }

protected:
    void onNewConnection() {
        auto socket = nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &FileTransfer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

        QFile file;
        file.setFileName(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qCriticalEx() << "Failed to open file for sending.";
            return;
        }

        qint64 bytesSent = 0;
        while (!file.atEnd()) {
            auto buffer = file.read(4096);
            qint64 written = socket->write(buffer);
            if (written == -1) {
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

    void onReadyRead() {
        QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
        
    }

private:
    QString path;
    QString md5;
    qint64 size;
};
