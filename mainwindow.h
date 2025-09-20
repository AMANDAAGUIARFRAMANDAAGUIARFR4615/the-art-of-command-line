#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QGridLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addItem();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void onTabClicked(int index);

    QWidget* bottomWidget;
    QGridLayout* gridLayout;
    int rowHeight;
    int totalCols;
};

#endif // MAINWINDOW_H
