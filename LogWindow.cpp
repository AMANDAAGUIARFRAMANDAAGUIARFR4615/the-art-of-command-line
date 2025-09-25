#include "logwindow.h"

#include <QVBoxLayout>

LogWindow* LogWindow::m_instance = nullptr;

LogWindow::LogWindow(QWidget *parent) : QWidget(parent), logText(new QTextBrowser(this))
{
    m_instance = this;
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(logText);
    setLayout(layout);
    resize(600, 400);

    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &message) {
        QMetaObject::invokeMethod(LogWindow::getInstance(), [message]() {
            LogWindow::getInstance()->append(message);
        });
    });
}

LogWindow* LogWindow::getInstance()
{
    return m_instance;
}

void LogWindow::append(const QString &text)
{
    logText->append(text);
}

void LogWindow::toggleVisibility()
{
    setVisible(!isVisible());
}
