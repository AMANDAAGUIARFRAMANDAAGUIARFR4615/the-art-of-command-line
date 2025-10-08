#include "RemoteDevice.h"
#include "Logger.h"
#include "ControlWindow.h"
#include "TcpServer.h"
#include "EventHub.h"
#include <QMediaPlayer>
#include <QString>
#include <QStyle>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QMouseEvent>

ControlWindow::ControlWindow(QTcpSocket* socket, DeviceInfo* const deviceInfo, QWidget *parent) : socket(socket), deviceInfo(deviceInfo), VideoFrameWidget(new QMediaPlayer(parent), parent)
{
    setAttribute(Qt::WA_InputMethodEnabled, true);

    EventHub::StartListening("orientation", [this](const QJsonValue &data, QTcpSocket* socket) {
        if (this->socket != socket)
            return;

        orientationChanged(data.toInt());
    });
}

ControlWindow::~ControlWindow()
{

}

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
        auto width = this->width() / deviceInfo->scaleFactor, height = this->height() / deviceInfo->scaleFactor;

        QJsonObject dataObject;
        dataObject["type"] = type;

        if (deviceInfo->orientation == 1) { // Portrait
            dataObject["x"] = x;
            dataObject["y"] = y;
        } else if (deviceInfo->orientation == 2) { // PortraitUpsideDown
            dataObject["x"] = width - x;
            dataObject["y"] = height - y;
        } else if (deviceInfo->orientation == 3) { // LandscapeRight
            dataObject["x"] = height - y;
            dataObject["y"] = x;
        } else if (deviceInfo->orientation == 4) { // LandscapeLeft
            dataObject["x"] = y;
            dataObject["y"] = width - x;
        }

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

    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
    {
        QJsonObject dataObject;
        dataObject["type"] = "keyPress";
        dataObject["key"] = keyText;

        QJsonObject jsonObject;
        jsonObject["event"] = "keyboard";
        jsonObject["data"] = dataObject;

        TcpServer::sendData(socket, jsonObject);
    }

    QWidget::keyPressEvent(event);
}

void ControlWindow::keyReleaseEvent(QKeyEvent *event)
{
    auto keyText = QKeySequence(event->key()).toString();

    qDebugEx() << "Key Released:" << keyText;

    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
    {
        QJsonObject dataObject;
        dataObject["type"] = "keyRelease";
        dataObject["key"] = keyText;

        QJsonObject jsonObject;
        jsonObject["event"] = "keyboard";
        jsonObject["data"] = dataObject;

        TcpServer::sendData(socket, jsonObject);
    }

    QWidget::keyReleaseEvent(event);
}

void ControlWindow::inputMethodEvent(QInputMethodEvent *event)
{
    QString commitText = event->commitString();
    if (!commitText.isEmpty())
    {
        qDebugEx() << "输入内容:" << commitText;

        QJsonObject jsonObject;
        jsonObject["event"] = "inputText";
        jsonObject["data"] = commitText;

        TcpServer::sendData(socket, jsonObject);
    }

    QWidget::inputMethodEvent(event);
}
