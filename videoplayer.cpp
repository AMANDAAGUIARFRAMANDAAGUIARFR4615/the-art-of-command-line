#include "VideoPlayer.h"
#include "Logger.h"
#include "ToastWidget.h"
#include "TestWindow.h"

#include <QMediaPlayer>
#include <QString>
#include <QStyle>
#include <QVideoWidget>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMediaMetaData>

VideoPlayer::VideoPlayer(QTcpSocket* socket, QWidget *parent) : socket(socket), QWidget(parent)
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

        if (videoWidth == 0 || videoHeight == 0) {
            auto resolution = m_mediaPlayer->metaData().value(QMediaMetaData::Resolution).toSize();
            if(resolution.isValid()){
                videoWidth = resolution.width();
                videoHeight = resolution.height();
                qDebug() << "视频宽高:" << videoWidth << videoHeight;
            } else {
                qDebug() << "无法获取视频宽高";
            }
        }

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

    auto *win = new TestWindow(socket);
    win->resize(videoWidth, videoHeight);
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->setSource(m_mediaPlayer->source());
    win->play();
    win->show();
}
