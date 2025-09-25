#pragma once

#include <QVideoWidget>
#include <QWidget>
#include <QUrl>

class QMediaPlayer;

class TestWindow : public QVideoWidget
{
    Q_OBJECT
public:
    explicit TestWindow(QWidget *parent = nullptr);
    ~TestWindow();

    void setSource(const QUrl &source);
    void play();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QMediaPlayer *m_mediaPlayer = nullptr;
};
