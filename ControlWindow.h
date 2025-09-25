#pragma once

#include <QVideoWidget>
#include <QWidget>
#include <QUrl>
#include <QEvent>
#include <QTouchEvent>
#include <QTcpSocket>

class QMediaPlayer;

class ControlWindow : public QVideoWidget
{
    Q_OBJECT
public:
    explicit ControlWindow(QTcpSocket* socket, QWidget *parent = nullptr);
    ~ControlWindow();

    void setSource(const QUrl &source);
    void play();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    QTcpSocket* socket;
    QMediaPlayer *m_mediaPlayer = nullptr;
};
