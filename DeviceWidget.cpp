#include "DeviceWidget.h"
#include "DeviceWindow.h"
#include <QVBoxLayout>
#include <QLabel>

DeviceWidget::DeviceWidget(QTcpSocket* socket, DeviceInfo* deviceInfo, QWidget *parent): DeviceView(socket, deviceInfo, parent)
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

    auto window = new DeviceWindow(socket, deviceInfo);
    window->getVideoFrameWidget()->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    window->getVideoFrameWidget()->setFixedSize(deviceInfo->screenWidth * deviceInfo->scaleFactor, deviceInfo->screenHeight * deviceInfo->scaleFactor);
    window->getVideoFrameWidget()->orientationChanged(deviceInfo->orientation);
    window->getVideoFrameWidget()->mediaPlayer->setSource(videoFrameWidget->mediaPlayer->source());
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
}
