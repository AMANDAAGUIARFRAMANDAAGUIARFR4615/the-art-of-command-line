#pragma once

#include "DeviceView.h"
#include "DeviceWidget.h"

class DeviceWindow : public DeviceView
{
    Q_OBJECT
public:
    explicit DeviceWindow(QTcpSocket* socket, DeviceInfo* deviceInfo, DeviceWidget* deviceWidget);
    ~DeviceWindow();

    DeviceWidget* const deviceWidget;

protected:
    QPointF getTransformedPosition(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *event) override;
};
