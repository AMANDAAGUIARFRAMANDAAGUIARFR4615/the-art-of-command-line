#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QWidget>
#include <QTextBrowser>

class LogWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent = nullptr);

    static LogWindow* getInstance();
    void append(const QString &text);
    void toggleVisibility();

private:
    static LogWindow* m_instance;

    QTextBrowser *logText;
};

#endif // LOGWINDOW_H
