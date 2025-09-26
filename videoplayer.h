#pragma once

#include "DeviceInfo.h"
#include <QWidget>
#include <QUrl>
#include <QTcpSocket>

class QMediaPlayer;

class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPlayer(QTcpSocket* socket, const DeviceInfo* deviceInfo, QWidget *parent = nullptr);
    ~VideoPlayer();

    void setSource(const QUrl &source);
    void play();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    QTcpSocket* socket;
    const DeviceInfo* deviceInfo;
    QMediaPlayer *m_mediaPlayer;
};
