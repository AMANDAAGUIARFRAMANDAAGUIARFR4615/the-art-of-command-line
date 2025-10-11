#include "DeviceWidget.h"
#include "DeviceWindow.h"
#include <QVBoxLayout>
#include <QLabel>

DeviceWidget::DeviceWidget(QTcpSocket* socket, DeviceInfo* deviceInfo): DeviceView(socket, deviceInfo)
{
    auto layout = new QVBoxLayout;

    auto deviceInfoText = QString("%1 - %2  |  %3 x %4")
        .arg(deviceInfo->deviceName)
        .arg(deviceInfo->platform)
        .arg(deviceInfo->screenWidth)
        .arg(deviceInfo->screenHeight);

    auto deviceInfoLabel = new QLabel(deviceInfoText, this);
    deviceInfoLabel->setAlignment(Qt::AlignCenter);
    deviceInfoLabel->setStyleSheet(
        "font-size: 12px; "
        "font-weight: bold; "
        "padding: 5px; "
        "background-color: rgba(0, 0, 0, 50%); "
        "color: white; "
        "border-radius: 5px;");
    
    deviceInfoLabel->setFixedHeight(24);
    
    layout->addWidget(deviceInfoLabel);
    setLayout(layout);
}

DeviceWidget::~DeviceWidget()
{

}

void DeviceWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);

    deviceWindow = new DeviceWindow(socket, deviceInfo, this);
    videoFrameWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    videoFrameWidget->setFixedSize(deviceInfo->screenWidth * deviceInfo->scaleFactor, deviceInfo->screenHeight * deviceInfo->scaleFactor);
    videoFrameWidget->orientationChanged(deviceInfo->orientation);
    deviceWindow->setAttribute(Qt::WA_DeleteOnClose);
    deviceWindow->show();

    addOverlay("设备控制中");
}
