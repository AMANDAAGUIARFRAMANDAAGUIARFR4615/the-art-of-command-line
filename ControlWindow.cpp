#include "RemoteDevice.h"
#include "Logger.h"
#include "ControlWindow.h"
#include "TcpServer.h"
#include <QMediaPlayer>
#include <QString>
#include <QStyle>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QMouseEvent>

ControlWindow::ControlWindow(QTcpSocket* socket, const DeviceInfo* deviceInfo, QWidget *parent) : socket(socket), deviceInfo(deviceInfo), VideoFrameWidget(new QMediaPlayer(parent), parent)
{
        
}

ControlWindow::~ControlWindow() {}

void ControlWindow::setSource(const QUrl &source)
{
    m_mediaPlayer->setSource(source);
}

void ControlWindow::play()
{
    m_mediaPlayer->play();
}

void ControlWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);

    qDebugEx() << "双击";
}

bool ControlWindow::event(QEvent *event)
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
        auto x = pos.x() / deviceInfo->scaleFactor, y = pos.y() / deviceInfo->scaleFactor;
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

void ControlWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        close();
        return;
    }

    auto keyText = QKeySequence(event->key()).toString();

    qDebugEx() << "Key Pressed:" << keyText;

    if (event->key() != Qt::Key_Backspace && event->key() != Qt::Key_Delete)
        return;

    QJsonObject dataObject;
    dataObject["type"] = "keyPress";
    dataObject["key"] = keyText;

    QJsonObject jsonObject;
    jsonObject["event"] = "keyboard";
    jsonObject["data"] = dataObject;

    TcpServer::sendData(socket, jsonObject);

    QWidget::keyPressEvent(event);
}

void ControlWindow::keyReleaseEvent(QKeyEvent *event)
{
    auto keyText = QKeySequence(event->key()).toString();

    qDebugEx() << "Key Released:" << keyText;

    if (event->key() != Qt::Key_Backspace && event->key() != Qt::Key_Delete)
        return;

    QJsonObject dataObject;
    dataObject["type"] = "keyRelease";
    dataObject["key"] = keyText;

    QJsonObject jsonObject;
    jsonObject["event"] = "keyboard";
    jsonObject["data"] = dataObject;

    TcpServer::sendData(socket, jsonObject);

    QWidget::keyReleaseEvent(event);
}
