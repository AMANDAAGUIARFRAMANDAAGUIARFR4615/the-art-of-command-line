#include "logwindow.h"

#include <QVBoxLayout>

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QString logMessage;
    switch (type) {
    case QtDebugMsg:
        logMessage = QString("[DEBUG] %1").arg(message);
        break;
    case QtInfoMsg:
        logMessage = QString("[INFO] %1").arg(message);
        break;
    case QtWarningMsg:
        logMessage = QString("[WARNING] %1").arg(message);
        break;
    case QtCriticalMsg:
        logMessage = QString("[CRITICAL] %1").arg(message);
        break;
    case QtFatalMsg:
        logMessage = QString("[FATAL] %1").arg(message);
        break;
    }

    QMetaObject::invokeMethod(LogWindow::getInstance(), [logMessage]() {
        LogWindow::getInstance()->append(logMessage);
    });
}

LogWindow* LogWindow::m_instance = nullptr;

LogWindow::LogWindow(QWidget *parent) : QWidget(parent), logText(new QTextBrowser(this))
{
    m_instance = this;
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(logText);
    setLayout(layout);
    resize(600, 400);

    qInstallMessageHandler(messageHandler);
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
