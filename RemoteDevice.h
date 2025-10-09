#pragma once

#include "DeviceInfo.h"
#include <QWidget>
#include <QUrl>
#include <QTcpSocket>

class QMediaPlayer;

class RemoteDevice : public QWidget
{
    Q_OBJECT
public:
    explicit RemoteDevice(QTcpSocket* socket, DeviceInfo* deviceInfo, QWidget *parent = nullptr);
    ~RemoteDevice();

    void setSource(const QUrl &source);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    QTcpSocket* socket;
    DeviceInfo* deviceInfo;
    QMediaPlayer *mediaPlayer;
};
