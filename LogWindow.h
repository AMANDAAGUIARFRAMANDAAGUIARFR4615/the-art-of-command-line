#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QWidget>
#include <QTextBrowser>

class LogWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent = nullptr);

    void append(const QString &text);
    void toggleVisibility();

private:
    QTextBrowser *logText;
};

#endif // LOGWINDOW_H
