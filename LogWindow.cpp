#include "logwindow.h"

#include <QVBoxLayout>

LogWindow* logWindow = nullptr;

LogWindow::LogWindow(QWidget *parent) : QWidget(parent), logText(new QTextBrowser(this))
{
    logWindow = this;
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(logText);
    setLayout(layout);
    resize(600, 400);

    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &message) {
        QMetaObject::invokeMethod(logWindow, [message]() {
            logWindow->append(message);
        });
    });
}

void LogWindow::append(const QString &text)
{
    logText->append(text);
}

void LogWindow::toggleVisibility()
{
    setVisible(!isVisible());
}
