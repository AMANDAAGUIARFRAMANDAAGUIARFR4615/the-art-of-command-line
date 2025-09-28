#pragma once

#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoFrame>
#include <QImage>
#include <QBuffer>
#include <QAbstractVideoBuffer>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QCryptographicHash>

class VideoFrameCapture : public QWidget
{
    Q_OBJECT

public:
    explicit VideoFrameCapture(QMediaPlayer *player, QWidget *parent = nullptr) : QWidget(parent), m_player(player)
    {
        m_videoSink = new QVideoSink(this);
        m_player->setVideoSink(m_videoSink);

        connect(m_videoSink, &QVideoSink::videoFrameChanged, this, &VideoFrameCapture::onVideoFrameChanged);

        m_player->setAudioOutput(nullptr);

        connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
            qDebug() << "Media Status Changed: " << status;
            if (status == QMediaPlayer::LoadedMedia) {
                if (!m_player->isPlaying()) {
                    m_player->stop();
                    m_player->play();
                }
            }
        });
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QWidget::paintEvent(event);

        if (!m_currentImage.isNull())
        {
            QSize widgetSize = size();
            QSize imageSize = m_currentImage.size();

            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing, true);

            int offsetX = (widgetSize.width() - imageSize.width()) / 2;
            int offsetY = (widgetSize.height() - imageSize.height()) / 2;

            painter.drawImage(offsetX, offsetY, m_currentImage);
        }
    }

    void onVideoFrameChanged(const QVideoFrame &frame)
    {
        if (!frame.isValid())
        {
            qDebug() << "无效帧";
            return;
        }

        QVideoFrame &nonConstFrame = const_cast<QVideoFrame &>(frame);

        nonConstFrame.map(QVideoFrame::ReadOnly);
        
        if (!canUpdateFrame(nonConstFrame))
        {
            qDebug() << "画面无变化";
            return;
        }
                
        const uchar *yPlane = nonConstFrame.bits(0);
        const uchar *uPlane = nonConstFrame.bits(1);
        const uchar *vPlane = nonConstFrame.bits(2);

        int width = frame.width();
        int height = frame.height();
        int yStride = nonConstFrame.bytesPerLine(0);
        int uvStride = nonConstFrame.bytesPerLine(1);

        QImage img(width, height, QImage::Format_RGB888);

        for (int y = 0; y < height; ++y)
        {
            uchar *line = img.scanLine(y);

            for (int x = 0; x < width; ++x)
            {
                int yValue = yPlane[y * yStride + x];
                int uIndex = (y / 2) * uvStride + (x / 2);
                int uValue = uPlane[uIndex];
                int vIndex = (y / 2) * uvStride + (x / 2);
                int vValue = vPlane[vIndex];

                int c = yValue - 16;
                int d = uValue - 128;
                int e = vValue - 128;
                int r = qBound(0, (298 * c + 409 * e + 128) >> 8, 255);
                int g = qBound(0, (298 * c - 100 * d - 208 * e + 128) >> 8, 255);
                int b = qBound(0, (298 * c + 516 * d + 128) >> 8, 255);

                line[3 * x]     = r;
                line[3 * x + 1] = g;
                line[3 * x + 2] = b;
            }
        }

        m_currentImage = img;

        update();

        nonConstFrame.unmap();
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

    QMediaPlayer *m_player;
    QVideoSink *m_videoSink;
    QImage m_currentImage;
    QByteArray m_lastFrameHash;
};