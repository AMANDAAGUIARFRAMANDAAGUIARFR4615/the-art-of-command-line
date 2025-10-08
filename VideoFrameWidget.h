#pragma once

#include "Logger.h"
#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoFrame>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDropEvent>

class VideoFrameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoFrameWidget(QMediaPlayer *player, QWidget *parent = nullptr) 
        : QWidget(parent), m_mediaPlayer(player)
    {
        videoSink = new QVideoSink(this);
        m_mediaPlayer->setVideoSink(videoSink);

        connect(videoSink, &QVideoSink::videoFrameChanged, this, &VideoFrameWidget::onVideoFrameChanged);

        m_mediaPlayer->setAudioOutput(nullptr);

        connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
            qDebugEx() << "Media Status Changed: " << status;
            if (status == QMediaPlayer::LoadedMedia) {
                if (!m_mediaPlayer->isPlaying()) {
                    qDebugEx() << "播放...";
                    m_mediaPlayer->stop();
                    m_mediaPlayer->play();
                }
            }
        });

        connect(m_mediaPlayer, &QMediaPlayer::errorChanged, [this]() {
            qCriticalEx() << "errorChanged" << m_mediaPlayer->errorString();
        });
    }

    void orientationChanged(int orientation, bool shouldResize = true)
    {
        switch (orientation) {
            case 1: // Portrait
                rotationAngle = 0;
                break;
            case 2: // PortraitUpsideDown
                rotationAngle = 180;
                break;
            case 3: // LandscapeRight
                rotationAngle = -90;
                break;
            case 4: // LandscapeLeft
                rotationAngle = 90;
                break;
        }

        auto width = size().width();
        auto height = size().height();

        if (shouldResize && ((orientation == 1 || orientation == 2) && height < width || (orientation == 3 || orientation == 4) && height > width))
            resize(height, width);
        else
            update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QWidget::paintEvent(event);

        if (currentImage.isNull())
            return;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        QTransform transform;
        transform.rotate(rotationAngle);

        auto rotatedImage = currentImage.transformed(transform);
        auto scaled = rotatedImage.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // 计算居中位置
        QRect targetRect(QPoint(0, 0), scaled.size());
        targetRect.moveCenter(rect().center());

        painter.drawImage(targetRect.topLeft(), scaled);
    }

    void onVideoFrameChanged(const QVideoFrame &frame)
    {
        if (!frame.isValid())
        {
            qCriticalEx() << "无效帧";
            return;
        }

        QImage img = frame.toImage();
        if (img.isNull())
        {
            qCriticalEx() << "frame.toImage()";
            return;
        }

        currentImage = img;
        update();
    }

    QMediaPlayer *m_mediaPlayer;
    QVideoSink *videoSink;
    QImage currentImage;
    int rotationAngle = 0;
};
