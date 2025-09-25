#include "VideoPlayer.h"
#include "Logger.h"
#include "ToastWidget.h"
#include "TestWindow.h"

#include <QMediaPlayer>
#include <QString>
#include <QStyle>
#include <QVideoWidget>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QMouseEvent>

TestWindow::TestWindow(QWidget *parent) : QVideoWidget(parent)
{
    m_mediaPlayer = new QMediaPlayer(this);

    // auto *videoWidget = new QVideoWidget(this);
    // videoWidget->setGeometry(rect()); // 初始大小填满窗口
    // videoWidget->show();
    // auto *layout = new QVBoxLayout;
    // layout->addWidget(videoWidget);
    // setLayout(layout);

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

    auto *win = new TestWindow;
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->show();
}
