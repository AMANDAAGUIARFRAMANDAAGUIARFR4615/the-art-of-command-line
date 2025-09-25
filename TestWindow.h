#pragma once

#include <QVideoWidget>
#include <QWidget>
#include <QUrl>
#include <QEvent>
#include <QTouchEvent>
#include <QTcpSocket>

class QMediaPlayer;

class TestWindow : public QVideoWidget
{
    Q_OBJECT
public:
    explicit TestWindow(QTcpSocket* socket, QWidget *parent = nullptr);
    ~TestWindow();

    void setSource(const QUrl &source);
    void play();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;

    QTcpSocket* socket;
    QMediaPlayer *m_mediaPlayer = nullptr;
};
