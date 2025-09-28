#pragma once

#include "DeviceInfo.h"
#include "VideoFrameWidget.h"
#include <QWidget>
#include <QUrl>
#include <QEvent>
#include <QTouchEvent>
#include <QTcpSocket>

class QMediaPlayer;

class ControlWindow : public VideoFrameWidget
{
    Q_OBJECT
public:
    explicit ControlWindow(QTcpSocket* socket, const DeviceInfo* deviceInfo, QWidget *parent = nullptr);
    ~ControlWindow();

    void setSource(const QUrl &source);
    void play();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    QTcpSocket* socket;
    const DeviceInfo* deviceInfo;
    QMediaPlayer *m_mediaPlayer;
};
