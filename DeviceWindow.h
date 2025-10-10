#pragma once

#include "DeviceView.h"

class DeviceWindow : public DeviceView
{
    Q_OBJECT
public:
    explicit DeviceWindow(QTcpSocket* socket, DeviceInfo* const deviceInfo, QWidget *parent = nullptr);
    ~DeviceWindow();

protected:
    QPointF getTransformedPosition(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *event) override;
};
