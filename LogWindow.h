#pragma once

#include <QTextBrowser>
#include <QMetaObject>
#include <QMessageLogContext>
#include <QTextBlock>

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
                if (type == QtCriticalMsg || type == QtFatalMsg || type == QtWarningMsg) {
                    logWindow->appendWithLimit(QString("<span style='color:red;'>%1</span>").arg(message));
                } else {
                    logWindow->appendWithLimit(message);
                }
            });
        });
    }

    void toggleVisibility()
    {
        setVisible(!isVisible());
    }

private:
    inline static LogWindow* logWindow;

    void appendWithLimit(const QString& message)
    {
        if (document()->blockCount() > 500) {
            QTextBlock firstBlock = document()->firstBlock();
            QTextCursor cursor(firstBlock);
            
            cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, 1);
            cursor.removeSelectedText();
        }

        append(message);
    }
};
