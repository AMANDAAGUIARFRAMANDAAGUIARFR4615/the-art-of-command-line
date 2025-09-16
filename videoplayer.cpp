#include "VideoPlayer.h"
#include "ToastWidget.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSizePolicy>
#include <QSlider>
#include <QStandardPaths>
#include <QString>
#include <QStyle>
#include <QVideoWidget>
#include <QElapsedTimer>

VideoPlayer::VideoPlayer(QWidget *parent) : QWidget(parent)
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

        const QString errorString = m_mediaPlayer->errorString();
        QString message = "Error: ";
        if (errorString.isEmpty())
            message += " #" + QString::number(int(m_mediaPlayer->error()));
        else
            message += errorString;

        new ToastWidget(message);
    });

    QElapsedTimer *timer = new QElapsedTimer;
    timer->start();

    bool isMediaLoaded = false;

    QObject::connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged,
                     [&isMediaLoaded, timer, this](QMediaPlayer::MediaStatus status) {
                         if (isMediaLoaded)
                             return;

                         qDebug() << "媒体加载中... " << status;
                         qDebug() << "耗时:" << timer->elapsed() << "ms";

                         if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {
                             isMediaLoaded = true;
                             qDebug() << "媒体加载完成，可以播放";
                             m_mediaPlayer->stop();
                             m_mediaPlayer->play();
                         }
                     });
}

VideoPlayer::~VideoPlayer() {}

void VideoPlayer::setUrl(const QUrl &url)
{
    m_mediaPlayer->setSource(url);
}

void VideoPlayer::play()
{
    m_mediaPlayer->play();
}
