#pragma once

#include <QWidget>
#include <QUrl>

class QMediaPlayer;

class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

    void setSource(const QUrl &source);
    void play();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QMediaPlayer *m_mediaPlayer = nullptr;
};
