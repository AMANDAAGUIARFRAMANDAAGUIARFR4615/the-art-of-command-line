// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "videoplayer.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSizePolicy>
#include <QSlider>
#include <QStandardPaths>
#include <QString>
#include <QStyle>
#include <QVideoWidget>
#include <QElapsedTimer>

VideoPlayer::VideoPlayer(QWidget *parent) : QWidget(parent)
{
    m_mediaPlayer = new QMediaPlayer(this);
    QVideoWidget *videoWidget = new QVideoWidget;

    m_errorLabel = new QLabel;
    m_errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(videoWidget);
    layout->addWidget(m_errorLabel);

    setLayout(layout);

    m_mediaPlayer->setVideoOutput(videoWidget);
    connect(m_mediaPlayer, &QMediaPlayer::errorChanged, this, &VideoPlayer::handleError);

    QElapsedTimer *timer = new QElapsedTimer;
    timer->start();

    QObject::connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged,
                     [this, timer](QMediaPlayer::MediaStatus status)
                     {
                         qDebug() << "媒体加载中... " << status;
                         qDebug() << "耗时:" << timer->elapsed() << "ms";

                         if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia)
                         {
                             qDebug() << "媒体加载完成，可以播放";
                             m_mediaPlayer->stop();
                             m_mediaPlayer->play();
                         }
                     });
}

VideoPlayer::~VideoPlayer() {}

void VideoPlayer::setUrl(const QUrl &url)
{
    m_errorLabel->setText(QString());
    m_mediaPlayer->setSource(url);
}

void VideoPlayer::play()
{
    m_mediaPlayer->play();
}

void VideoPlayer::handleError()
{
    if (m_mediaPlayer->error() == QMediaPlayer::NoError)
        return;

    const QString errorString = m_mediaPlayer->errorString();
    QString message = "Error: ";
    if (errorString.isEmpty())
        message += " #" + QString::number(int(m_mediaPlayer->error()));
    else
        message += errorString;
    m_errorLabel->setText(message);
}

#include "moc_videoplayer.cpp"
