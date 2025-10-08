#pragma once

#include <QTextBrowser>
#include <QMetaObject>
#include <QMessageLogContext>
#include <QStringList>

class LogWindow : public QTextBrowser
{
    Q_OBJECT
public:
    explicit LogWindow(QWidget *parent = nullptr) : QTextBrowser(parent)
    {
        logWindow = this;

        setVisible(false);

        qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &message) {
            QMetaObject::invokeMethod(logWindow, [type, message]() {
                if (logWindow->logEntries.size() >= 1000) {
                    logWindow->logEntries.removeFirst();
                }

                if (type == QtCriticalMsg || type == QtFatalMsg || type == QtWarningMsg) {
                    logWindow->logEntries.append(QString("<span style='color:red;'>%1</span>").arg(message));
                } else {
                    logWindow->logEntries.append(message);
                }

                logWindow->setPlainText(logWindow->logEntries.join("\n"));

                logWindow->moveCursor(QTextCursor::End);
            });
        });
    }

    void toggleVisibility()
    {
        setVisible(!isVisible());
    }

private:
    inline static LogWindow* logWindow;
    QStringList logEntries;
};
