#pragma once

#include <QTcpSocket>
#include <QTcpServer>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QTextStream>

class FileTransfer : public QTcpServer {
    Q_OBJECT
public:
    FileTransfer(int type, const QString& path, const QString& md5, int64 size) {
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

        file.setFileName(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open file for sending.";
            return;
        }

        while (!file.atEnd()) {
            auto buffer = file.read(4096);
            socket.write(buffer);
            socket.waitForBytesWritten();
        }

        file.close();   
    }

    void onClientConnected() {
        qDebug() << "Client connected, ready to send file.";
        // 假设你需要发送一个文件
        sendFile("path_to_file");
    }

    void onReadyRead() {
        QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
        if (!socket) return;

        QDataStream stream(socket);
        stream.setVersion(QDataStream::Qt_5_15);

        if (fileName.isEmpty()) {
            stream >> fileName;  // 接收文件名
            stream >> fileSize;   // 接收文件大小

            file.setFileName(fileName);
            if (!file.open(QIODevice::WriteOnly)) {
                qWarning() << "Failed to open file for receiving.";
                return;
            }
        }

        QByteArray data = socket->readAll();
        file.write(data);

        if (file.size() == fileSize) {
            qDebug() << "File transfer completed.";
            file.close();
        }
    }

private:
    QTcpSocket socket;
    QFile file;
    QString fileName;
    qint64 fileSize = 0;
};
