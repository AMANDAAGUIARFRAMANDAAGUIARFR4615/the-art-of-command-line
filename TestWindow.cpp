#include "VideoPlayer.h"
#include "Logger.h"
#include "ToastWidget.h"
#include "TestWindow.h"
#include "TcpServer.h"

#include <QMediaPlayer>
#include <QString>
#include <QStyle>
#include <QVideoWidget>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QMouseEvent>

TestWindow::TestWindow(QTcpSocket *socket, QWidget *parent) : socket(socket), QVideoWidget(parent)
{
    m_mediaPlayer = new QMediaPlayer(this);

    m_mediaPlayer->setVideoOutput(this);

    connect(m_mediaPlayer, &QMediaPlayer::errorChanged, [this]()
            {
        if (m_mediaPlayer->error() == QMediaPlayer::NoError)
            return;

        const auto errorString = m_mediaPlayer->errorString();
        QString message = "Error: ";
        if (errorString.isEmpty())
            message += " #" + QString::number(int(m_mediaPlayer->error()));
        else
            message += errorString;

        new ToastWidget(message); });

    auto *timer = new QElapsedTimer;
    timer->start();

    auto isMediaLoaded = false;

    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, [&isMediaLoaded, timer, this](QMediaPlayer::MediaStatus status)
            {
        if (isMediaLoaded)
            return;

        qDebugEx() << "媒体加载中... " << status;
        qDebugEx() << "耗时:" << timer->elapsed() << "ms";

        if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {
            isMediaLoaded = true;
            qDebugEx() << "媒体加载完成，可以播放";
            m_mediaPlayer->stop();
            m_mediaPlayer->play();
        } });
}

TestWindow::~TestWindow() {}

void TestWindow::setSource(const QUrl &source)
{
    m_mediaPlayer->setSource(source);
}

void TestWindow::play()
{
    m_mediaPlayer->play();
}

void TestWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);

    qDebugEx() << "双击";
}

bool TestWindow::event(QEvent *event)
{
    int type = 0;
    Qt::MouseButton button = Qt::NoButton;

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
        type = 1;
        button = static_cast<QMouseEvent *>(event)->button();
        break;
    case QEvent::MouseButtonRelease:
        type = 2;
        button = static_cast<QMouseEvent *>(event)->button();
        break;
    case QEvent::MouseMove:
        type = 3;
        break;

    default:
        break;
    }

    if ((type == 1 || type == 2) && button == Qt::LeftButton || type == 3)
    {
        auto pos = static_cast<QMouseEvent *>(event)->pos();
        auto x = pos.x(), y = pos.y();
        // qDebugEx() << "event" << event->type() << x << y;

        QJsonObject dataObject;
        dataObject["type"] = type;
        dataObject["x"] = x;
        dataObject["y"] = y;

        QJsonObject jsonObject;
        jsonObject["event"] = "mouse";
        jsonObject["data"] = dataObject;
        TcpServer::sendData(socket, jsonObject);
    }

    return QWidget::event(event);
}

void TestWindow::keyPressEvent(QKeyEvent *event)
{
    qDebugEx() << "Key Pressed:" << event->key();

    QJsonObject dataObject;
    dataObject["type"] = "keyPress";
    dataObject["key"] = event->key();

    QJsonObject jsonObject;
    jsonObject["event"] = "keyboard";
    jsonObject["data"] = dataObject;

    TcpServer::sendData(socket, jsonObject);

    QWidget::keyPressEvent(event);
}

void TestWindow::keyReleaseEvent(QKeyEvent *event)
{
    qDebugEx() << "Key Released:" << event->key();

    QJsonObject dataObject;
    dataObject["type"] = "keyRelease";
    dataObject["key"] = event->key();

    QJsonObject jsonObject;
    jsonObject["event"] = "keyboard";
    jsonObject["data"] = dataObject;

    TcpServer::sendData(socket, jsonObject);

    QWidget::keyReleaseEvent(event);
}
