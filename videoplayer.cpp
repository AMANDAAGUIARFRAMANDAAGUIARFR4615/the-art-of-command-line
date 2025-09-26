#include "VideoPlayer.h"
#include "Logger.h"
#include "ToastWidget.h"
#include "ControlWindow.h"
#include "DeviceControl.h"
#include "RemoteFileExplorerWidget.h"
#include <QMediaPlayer>
#include <QString>
#include <QStyle>
#include <QVideoWidget>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>

VideoPlayer::VideoPlayer(QTcpSocket* socket, const DeviceInfo* deviceInfo, QWidget *parent) : socket(socket), deviceInfo(deviceInfo), QWidget(parent)
{
    m_mediaPlayer = new QMediaPlayer(this);

    auto *videoWidget = new QVideoWidget;
    auto *layout = new QVBoxLayout;
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

    auto isMediaLoaded = false;

    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, [&isMediaLoaded, timer, this](QMediaPlayer::MediaStatus status) {
        if (isMediaLoaded)
            return;

        qDebugEx() << "媒体加载中... " << status;
        qDebugEx() << "耗时:" << timer->elapsed() << "ms";

        if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {
            isMediaLoaded = true;
            qDebugEx() << "媒体加载完成，可以播放";
            m_mediaPlayer->stop();
            m_mediaPlayer->play();
        }
    });
}

VideoPlayer::~VideoPlayer() {}

void VideoPlayer::setSource(const QUrl &source)
{
    m_mediaPlayer->setSource(source);
}

void VideoPlayer::play()
{
    m_mediaPlayer->play();
}

void VideoPlayer::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);

    auto *win = new ControlWindow(socket, deviceInfo);
    win->resize(deviceInfo->screenWidth, deviceInfo->screenHeight);
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->setSource(m_mediaPlayer->source());
    win->play();
    win->show();
}

void VideoPlayer::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu(this);

    QAction *fileAction = new QAction("文件传输", this);
    connect(fileAction, &QAction::triggered, [this]() {
        auto window = new RemoteFileExplorerWidget;
        window->resize(deviceInfo->screenWidth, deviceInfo->screenHeight);
        window->show();
    });

    QAction *unlockAction = new QAction("解锁", this);
    connect(unlockAction, &QAction::triggered, [this]() {
        DeviceControl::unlockScreen(socket);
    });

    QAction *lockAction = new QAction("锁屏", this);
    connect(lockAction, &QAction::triggered, [this]() {
        DeviceControl::lockScreen(socket);
    });

    QAction *rebootAction = new QAction("重启", this);
    connect(rebootAction, &QAction::triggered, [this]() {
        DeviceControl::reboot(socket);
    });

    contextMenu.addAction(fileAction);
    contextMenu.addAction(unlockAction);
    contextMenu.addAction(lockAction);
    contextMenu.addAction(rebootAction);

    contextMenu.exec(event->globalPos());
}
