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

QPointF ControlWindow::getTransformedPosition(QMouseEvent *event) {
    auto x = event->pos().x() / deviceInfo->scaleFactor;
    auto y = event->pos().y() / deviceInfo->scaleFactor;
    auto width = this->width() / deviceInfo->scaleFactor;
    auto height = this->height() / deviceInfo->scaleFactor;

    switch (deviceInfo->orientation) {
        case 1: // Portrait
            return QPointF(x, y);
        case 2: // PortraitUpsideDown
            return QPointF(width - x, height - y);
        case 3: // LandscapeRight
            return QPointF(height - y, x);
        case 4: // LandscapeLeft
            return QPointF(y, width - x);
        default:
            return QPointF(x, y);
    }
}

void ControlWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);

    qDebugEx() << "双击" << event->button();

    if (event->button() == Qt::LeftButton) {
        QMouseEvent *pressEvent = new QMouseEvent(QEvent::MouseButtonPress, event->pos(), event->button(), event->button(), event->modifiers());

        QMouseEvent *releaseEvent = new QMouseEvent(QEvent::MouseButtonRelease, event->pos(), event->button(), event->button(), event->modifiers());

        QApplication::postEvent(this, pressEvent);

        QTimer::singleShot(100, [this, releaseEvent]() {
            QApplication::postEvent(this, releaseEvent);
        });
    }
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
    }

    if ((type == 1 || type == 2) && button == Qt::LeftButton || type == 3)
    {
        auto pos = this->getTransformedPosition(static_cast<QMouseEvent *>(event));

        QJsonObject dataObject;
        dataObject["type"] = type;
        dataObject["x"] = pos.x();
        dataObject["y"] = pos.y();

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
