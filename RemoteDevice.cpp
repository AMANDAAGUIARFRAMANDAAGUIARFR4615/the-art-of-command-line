#include "RemoteDevice.h"
#include "Logger.h"
#include "ControlWindow.h"
#include "Tools.h"
#include "RemoteFileExplorer.h"
#include "TcpServer.h"
#include "FileTransfer.h"
#include "EventHub.h"
#include <QMediaPlayer>
#include <QString>
#include <QStyle>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QLabel>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QUuid>
#include <QFileInfo>

RemoteDevice::RemoteDevice(QTcpSocket* socket, DeviceInfo* deviceInfo, QWidget *parent)
    : socket(socket), deviceInfo(deviceInfo), QWidget(parent)
{
    setAcceptDrops(true);

    videoFrameWidget = new VideoFrameWidget(this);
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
    layout->addWidget(videoFrameWidget);
    setLayout(layout);

    videoFrameWidget->orientationChanged(deviceInfo->orientation);

    EventHub::StartListening("orientation", [this](const QJsonValue &data, QTcpSocket* socket) {
        if (this->socket != socket)
            return;

        this->deviceInfo->orientation = data.toInt();
        videoFrameWidget->orientationChanged(data.toInt());
    });

    lastSource = videoFrameWidget->mediaPlayer->source();

    EventHub::StartListening("lockedStatus", [this, deviceInfo](const QJsonValue &data, QTcpSocket* socket) {
        if (this->socket != socket)
            return;

        auto locked = data.toBool();

        if (locked)
        {
            lastSource = videoFrameWidget->mediaPlayer->source();
            this->layout()->removeWidget(videoFrameWidget);
            videoFrameWidget->deleteLater();
        }
        else
        {
            videoFrameWidget = new VideoFrameWidget(this);
            videoFrameWidget->mediaPlayer->setSource(lastSource);
            this->layout()->addWidget(videoFrameWidget);
        }
    });
}

RemoteDevice::~RemoteDevice()
{

}

void RemoteDevice::setSource(const QUrl &source)
{
    videoFrameWidget->mediaPlayer->setSource(source);
}

void RemoteDevice::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);

    auto window = new ControlWindow(socket, deviceInfo);
    window->videoFrameWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    window->videoFrameWidget->setFixedSize(deviceInfo->screenWidth * deviceInfo->scaleFactor, deviceInfo->screenHeight * deviceInfo->scaleFactor);
    window->videoFrameWidget->orientationChanged(deviceInfo->orientation);
    window->videoFrameWidget->mediaPlayer->setSource(videoFrameWidget->mediaPlayer->source());
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
}

void RemoteDevice::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu(this);

    QAction *fileAction = new QAction("文件传输", this);
    connect(fileAction, &QAction::triggered, [this]() {
        auto window = new RemoteFileExplorer(socket);
        window->resize(deviceInfo->screenWidth * deviceInfo->scaleFactor, deviceInfo->screenHeight * deviceInfo->scaleFactor);
        window->show();
    });

    QAction *unlockAction = new QAction("解锁", this);
    connect(unlockAction, &QAction::triggered, [this]() {
        Tools::unlockScreen(socket);
    });

    QAction *lockAction = new QAction("锁屏", this);
    connect(lockAction, &QAction::triggered, [this]() {
        Tools::lockScreen(socket);
    });

    QAction *rebootAction = new QAction("重启", this);
    connect(rebootAction, &QAction::triggered, [this]() {
        Tools::reboot(socket);
    });

    QAction *volumeUpAction = new QAction("音量+", this);
    connect(volumeUpAction, &QAction::triggered, [this]() {
        QJsonObject jsonObject;
        jsonObject["event"] = "volumeControl";
        jsonObject["data"] = "+";

        TcpServer::sendData(socket, jsonObject);
    });

    QAction *volumeDownAction = new QAction("音量-", this);
    connect(volumeDownAction, &QAction::triggered, [this]() {
        QJsonObject jsonObject;
        jsonObject["event"] = "volumeControl";
        jsonObject["data"] = "-";

        TcpServer::sendData(socket, jsonObject);
    });

    contextMenu.addAction(fileAction);
    contextMenu.addAction(unlockAction);
    contextMenu.addAction(lockAction);
    contextMenu.addAction(rebootAction);
    contextMenu.addAction(volumeUpAction);
    contextMenu.addAction(volumeDownAction);

    contextMenu.exec(event->globalPos());
}

void RemoteDevice::dragEnterEvent(QDragEnterEvent *event)
{
    qDebugEx() << "dragEnterEvent";

    const QList<QUrl> urls = event->mimeData()->urls();

    for (const QUrl &url : urls)
    {
        auto path = url.toLocalFile();
        auto fileName = QFileInfo(path).fileName();
        if (!fileName.endsWith(".deb", Qt::CaseInsensitive) && !fileName.endsWith(".ipa", Qt::CaseInsensitive))
        {
            event->ignore();
            return;
        }
    }

    event->accept();
}

void RemoteDevice::dropEvent(QDropEvent *event)
{
    qDebugEx() << "dropEvent";

    const QList<QUrl> urls = event->mimeData()->urls();

    for (const QUrl& url : urls) {
        auto id = QUuid::createUuid().toString();
        auto type = 2; // 收是1，发是2
        auto path = url.toLocalFile();
        auto size = Tools::getFileSize(path);
        
        auto transfer = new FileTransfer(type, path, size);

        auto fileName = QFileInfo(path).fileName();
        if (fileName.endsWith(".deb", Qt::CaseInsensitive)) {
            QJsonObject dataObject;
            dataObject["id"] = id;
            dataObject["type"] = type;
            dataObject["port"] = transfer->serverPort();
            dataObject["name"] = fileName;
            dataObject["size"] = size;

            QJsonObject jsonObject;
            jsonObject["event"] = "debInstall";
            jsonObject["data"] = dataObject;

            TcpServer::sendData(socket, jsonObject);
        }
    }

    event->accept();
}
