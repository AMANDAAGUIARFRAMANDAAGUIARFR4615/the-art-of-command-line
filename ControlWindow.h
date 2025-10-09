#pragma once

#include "DeviceInfo.h"
#include "VideoFrameWidget.h"
#include <QWidget>
#include <QUrl>
#include <QEvent>
#include <QTouchEvent>
#include <QTcpSocket>

class QMediaPlayer;

class ControlWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ControlWindow(QTcpSocket* socket, DeviceInfo* const deviceInfo, QWidget *parent = nullptr);
    ~ControlWindow();

    VideoFrameWidget* const videoFrameWidget;

protected:
    QPointF getTransformedPosition(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *event) override;

    QTcpSocket* socket;
    DeviceInfo* const deviceInfo;
};
