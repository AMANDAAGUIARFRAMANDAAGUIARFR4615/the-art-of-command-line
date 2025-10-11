#pragma once

#include "DeviceInfo.h"
#include "VideoFrameWidget.h"
#include <QWidget>
#include <QUrl>
#include <QTcpSocket>

class QMediaPlayer;

class DeviceView : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceView(QTcpSocket* socket, DeviceInfo* deviceInfo, QWidget *parent = nullptr);
    ~DeviceView();

    VideoFrameWidget* getVideoFrameWidget() { return videoFrameWidget; }
    void setSource(const QUrl &source);

    void addOverlay(const QString &text);
    void addVideoFrameWidget(VideoFrameWidget* videoFrameWidget);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    QTcpSocket* const socket;
    DeviceInfo* const deviceInfo;
    VideoFrameWidget *videoFrameWidget = nullptr;
    QUrl mediaSource;
};
