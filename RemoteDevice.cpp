#include "RemoteDevice.h"
#include "Logger.h"
#include "ToastWidget.h"
#include "ControlWindow.h"
#include "Tools.h"
#include "RemoteFileExplorer.h"
#include "TcpServer.h"
#include "FileTransfer.h"
#include <QMediaPlayer>
#include <QString>
#include <QStyle>
#include <QVideoWidget>
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

RemoteDevice::RemoteDevice(QTcpSocket* socket, const DeviceInfo* deviceInfo, QWidget *parent) 
    : socket(socket), deviceInfo(deviceInfo), QWidget(parent)
{
    setAcceptDrops(true);

    m_mediaPlayer = new QMediaPlayer(this);

    auto *videoWidget = new QVideoWidget;
    auto *layout = new QVBoxLayout;

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
    layout->addWidget(videoWidget);
    setLayout(layout);

    m_mediaPlayer->setVideoOutput(videoWidget);

    connect(m_mediaPlayer, &QMediaPlayer::errorChanged, [this]() {
        if (m_mediaPlayer->error() == QMediaPlayer::NoError)
            return;

        const auto errorString = m_mediaPlayer->errorString();
        QString message = "Error: ";
        if (errorString.isEmpty())
            message += " #" + QString::number(int(m_mediaPlayer->error()));
        else
            message += errorString;

        new ToastWidget(message);
    });

    auto *timer = new QElapsedTimer;
    timer->start();

    auto isMediaLoaded = new bool(false);

    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, [isMediaLoaded, timer, this](QMediaPlayer::MediaStatus status) {
        if (*isMediaLoaded)
            return;

        qDebugEx() << "媒体加载中... " << status;
        qDebugEx() << "耗时:" << timer->elapsed() << "ms";

        if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {
            *isMediaLoaded = true;
            qDebugEx() << "媒体加载完成，可以播放";
            m_mediaPlayer->stop();
            m_mediaPlayer->play();
        }
    });
}

RemoteDevice::~RemoteDevice() {}

void RemoteDevice::setSource(const QUrl &source)
{
    m_mediaPlayer->setSource(source);
}

void RemoteDevice::play()
{
    m_mediaPlayer->play();
}

void RemoteDevice::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);

    auto *window = new ControlWindow(socket, deviceInfo);
    window->resize(deviceInfo->screenWidth * deviceInfo->scaleFactor, deviceInfo->screenHeight * deviceInfo->scaleFactor);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setSource(m_mediaPlayer->source());
    window->play();
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

    contextMenu.addAction(fileAction);
    contextMenu.addAction(unlockAction);
    contextMenu.addAction(lockAction);
    contextMenu.addAction(rebootAction);

    contextMenu.exec(event->globalPos());
}

void RemoteDevice::dragEnterEvent(QDragEnterEvent *event)
{
    qDebugEx() << "dragEnterEvent";

    if (event->mimeData()->hasUrls()) 
        event->accept();
    else
        event->ignore();
}

void RemoteDevice::dropEvent(QDropEvent *event)
{
    qDebugEx() << "dropEvent";

    // 获取拖放的文件路径
    const QList<QUrl> urls = event->mimeData()->urls();

    for (const QUrl& url : urls) {
        auto type = 2;//收是1，发是2
        auto path = url.toLocalFile();
        auto md5 = Tools::getFileMd5(path);
        auto size = Tools::getFileSize(path);
        
        qDebugEx() << "拖放的文件路径: " << path << "md5:" << md5 << "size:" << size;

        auto transfer = new FileTransfer(type, path, md5, size);

        QJsonObject dataObject;
        dataObject["type"] = 2;//收是1，发是2
        dataObject["port"] = transfer->serverPort();
        dataObject["path"] = QString("/usr/") + md5;
        dataObject["md5"] = md5;
        dataObject["size"] = size;

        QJsonObject jsonObject;
        jsonObject["event"] = "transferFile";
        jsonObject["data"] = dataObject;

        TcpServer::sendData(socket, jsonObject);
    }

    event->accept();
}
