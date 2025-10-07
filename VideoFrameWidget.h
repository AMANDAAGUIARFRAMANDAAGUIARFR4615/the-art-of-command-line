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
#include <QCryptographicHash>

class VideoFrameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoFrameWidget(QMediaPlayer *player, QWidget *parent = nullptr) 
        : QWidget(parent), m_mediaPlayer(player)
    {
        m_videoSink = new QVideoSink(this);
        m_mediaPlayer->setVideoSink(m_videoSink);

        connect(m_videoSink, &QVideoSink::videoFrameChanged, this, &VideoFrameWidget::onVideoFrameChanged);

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

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QWidget::paintEvent(event);

        if (m_currentImage.isNull())
            return;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        // 保持比例缩放并平滑绘制
        auto scaled = m_currentImage.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

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

        m_currentImage = img;
        update();
    }

    bool canUpdateFrame(const QVideoFrame &frame)
    {
        // 使用哈希值判断帧是否发生变化
        QByteArray currentFrameHash = generateFrameHash(frame);
        if (currentFrameHash == m_lastFrameHash)
        {
            return false; 
        }

        m_lastFrameHash = currentFrameHash;
        return true;
    }

    QByteArray generateFrameHash(const QVideoFrame &frame)
    {
        // 通过获取 Y、U、V 平面和它们的字节数生成哈希
        QCryptographicHash hash(QCryptographicHash::Sha256);

        // 获取 Y、U、V 平面的数据
        const uchar *yPlane = frame.bits(0);
        const uchar *uPlane = frame.bits(1);
        const uchar *vPlane = frame.bits(2);

        // 计算每个平面的字节数
        int yBytes = frame.bytesPerLine(0) * frame.height();
        int uvBytes = frame.bytesPerLine(1) * (frame.height() / 2); // UV平面高度为原高度的一半

        // 将 Y、U、V 平面的数据加入哈希计算
        hash.addData(reinterpret_cast<const char*>(yPlane), yBytes);
        hash.addData(reinterpret_cast<const char*>(uPlane), uvBytes);
        hash.addData(reinterpret_cast<const char*>(vPlane), uvBytes);

        return hash.result();
    }

    QMediaPlayer *m_mediaPlayer;
    QVideoSink *m_videoSink;
    QImage m_currentImage;
    QByteArray m_lastFrameHash;
};
