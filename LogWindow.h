#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QTextBrowser>
#include <QMetaObject>
#include <QMessageLogContext>

class LogWindow : public QTextBrowser
{
    Q_OBJECT
public:
    explicit LogWindow(QWidget *parent = nullptr) : QTextBrowser(parent)
    {
        logWindow = this;

        qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &message) {
            QMetaObject::invokeMethod(logWindow, [type, message]() {
                if (type == QtCriticalMsg || type == QtFatalMsg || type == QtWarningMsg) {
                    // 红色显示错误、警告、致命信息
                    logWindow->append(QString("<span style='color:red;'>%1</span>").arg(message));
                } else {
                    // 普通信息默认黑色
                    logWindow->append(message);
                }
            });
        });
    }

    void toggleVisibility()
    {
        setVisible(!isVisible());
    }

private:
    inline static LogWindow* logWindow = nullptr;
};

#endif // LOGWINDOW_H
