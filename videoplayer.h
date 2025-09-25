#pragma once

#include <QWidget>
#include <QUrl>
#include <QTcpSocket>

class QMediaPlayer;

class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPlayer(QTcpSocket* socket, QWidget *parent = nullptr);
    ~VideoPlayer();

    void setSource(const QUrl &source);
    void play();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    QTcpSocket* socket;
    QMediaPlayer *m_mediaPlayer = nullptr;
    int videoWidth, videoHeight;
};
