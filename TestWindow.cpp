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

TestWindow::TestWindow(QTcpSocket* socket, QWidget *parent) : socket(socket), QVideoWidget(parent)
{
    m_mediaPlayer = new QMediaPlayer(this);

    m_mediaPlayer->setVideoOutput(this);

    connect(m_mediaPlayer, &QMediaPlayer::errorChanged, [this]() {
        if (m_mediaPlayer->error() == QMediaPlayer::NoError)
            return;

        const auto errorString = m_mediaPlayer->errorString();
        QString message = "Error: ";
        if (errorString.isEmpty())
            message += " #" + QString::number(int(m_mediaPlayer->error()));
        else
            message += errorString;

        new ToastWidget(message);
    });

    auto *timer = new QElapsedTimer;
    timer->start();

    auto isMediaLoaded = false;

    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, [&isMediaLoaded, timer, this](QMediaPlayer::MediaStatus status) {
        if (isMediaLoaded)
            return;

        qDebugEx() << "媒体加载中... " << status;
        qDebugEx() << "耗时:" << timer->elapsed() << "ms";

        if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {
            isMediaLoaded = true;
            qDebugEx() << "媒体加载完成，可以播放";
            m_mediaPlayer->stop();
            m_mediaPlayer->play();
        }
    });
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

bool TestWindow::event(QEvent *event)  {
    int type = 0;
    switch (event->type()) {
        case QEvent::MouseButtonPress:
            type = 1;
        case QEvent::MouseButtonRelease:
            type = 2;
        case QEvent::MouseMove:
            type = 3;

            qDebugEx() << "event" << event->type();
            TcpServer::sendData(socket, QJsonObject{{"event", "mouse"}, {"data", type}});
            return true;
        default:
            break;
        }
        
        return QWidget::event(event);
    }

// 鼠标按下
// void TestWindow::mousePressEvent(QMouseEvent *event)
// {
//     if (event->button() == Qt::LeftButton) {
//         m_pressPos = event->pos();
//         m_longPressTimer->start();
//     }

//     QWidget::mousePressEvent(event);
// }

// // 鼠标释放
// void TestWindow::mouseReleaseEvent(QMouseEvent *event)
// {
//     if (event->button() == Qt::LeftButton) {
//         m_longPressTimer->stop();

//         // 判断是否是点击事件（移动距离小于阈值）
//         if ((event->pos() - m_pressPos).manhattanLength() < 5) {
//             qDebug() << "点击事件触发";
//             new ToastWidget("点击事件触发");
//         }
//     }

//     QWidget::mouseReleaseEvent(event);
// }

// // 鼠标移动
// void TestWindow::mouseMoveEvent(QMouseEvent *event)
// {
//     if ((event->pos() - m_pressPos).manhattanLength() > 5) {
//         m_longPressTimer->stop(); // 移动则取消长按
//         qDebug() << "鼠标移动: " << event->pos();
//     }

//     QWidget::mouseMoveEvent(event);
// }
