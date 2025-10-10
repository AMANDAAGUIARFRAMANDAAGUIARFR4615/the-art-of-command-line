#pragma once

#include "DeviceView.h"

class DeviceWidget : public DeviceView
{
    Q_OBJECT
public:
    explicit DeviceWidget(QTcpSocket* socket, DeviceInfo* deviceInfo, QWidget *parent = nullptr);
    ~DeviceWidget();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};
