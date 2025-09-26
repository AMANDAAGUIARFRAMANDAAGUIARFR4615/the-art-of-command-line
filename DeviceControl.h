#ifndef DEVICECONTROL_H
#define DEVICECONTROL_H

#include "TcpServer.h"

#include <QJsonObject>
#include <QTcpSocket>

class DeviceControl {
public:
    // 锁屏
    static void lockScreen(QTcpSocket* socket) {
        sendEvent(socket, "changeScreenLockedStatus", 1);
    }

    // 解锁
    static void unlockScreen(QTcpSocket* socket) {
        sendEvent(socket, "changeScreenLockedStatus", 0);
    }

    // 重启
    static void reboot(QTcpSocket* socket) {
        sendEvent(socket, "reboot", 1);
    }

private:
    static void sendEvent(QTcpSocket* socket, const QString& event, int data) {
        QJsonObject jsonObject;
        jsonObject["event"] = event;
        jsonObject["data"] = data;
        TcpServer::sendData(socket, jsonObject);
    }
};

#endif // DEVICECONTROL_H
