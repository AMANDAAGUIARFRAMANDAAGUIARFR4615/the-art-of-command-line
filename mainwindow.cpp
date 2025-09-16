#include "MainWindow.h"
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QGuiApplication>
#include <QScreen>
#include <QApplication>
#include <QTabBar>
#include "VideoPlayer.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // 上方大区域：Tab 标签页
    QTabWidget *tabWidget = new QTabWidget(this);
    QWidget *tab1 = new QWidget();
    QWidget *tab2 = new QWidget();
    QWidget *tab3 = new QWidget();

    tab1->setLayout(new QVBoxLayout());

    VideoPlayer *player = new VideoPlayer();
    player->setUrl(QString("tcp://192.168.0.102:23145"));

    tab1->layout()->addWidget(player);
    tab2->setLayout(new QVBoxLayout());
    tab2->layout()->addWidget(new QLabel("这是标签页 2 的内容"));
    tab3->setLayout(new QVBoxLayout());
    tab3->layout()->addWidget(new QLabel("这是标签页 3 的内容"));

    tabWidget->addTab(tab1, "Tab 1");
    tabWidget->addTab(tab2, "Tab 2");
    tabWidget->addTab(tab3, "Tab 3");

    connect(tabWidget->tabBar(), &QTabBar::tabBarClicked, this, &MainWindow::onTabClicked);

    // 下方 2 行 6 列的小布局区域
    QWidget *bottomWidget = new QWidget(this);
    QGridLayout *gridLayout = new QGridLayout(bottomWidget);

    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 6; ++col)
        {
            QFrame *frame = new QFrame(bottomWidget);
            frame->setFrameShape(QFrame::Box);
            frame->setLineWidth(2);

            QLabel *label = new QLabel(QString("区域 %1,%2").arg(row + 1).arg(col + 1), frame);
            label->setAlignment(Qt::AlignCenter);

            QVBoxLayout *frameLayout = new QVBoxLayout(frame);
            frameLayout->addWidget(label);

            gridLayout->addWidget(frame, row, col);
        }
    }

    // 上下布局，比例 2:1
    mainLayout->addWidget(tabWidget, 2);
    mainLayout->addWidget(bottomWidget, 1);

    setCentralWidget(central);
    setWindowTitle("Qt 多布局示例");

    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    resize(screenWidth * 0.8, screenHeight * 0.8);
}

MainWindow::~MainWindow()
{
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        QApplication::quit();
    }
    else
    {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::onTabClicked(int index)
{
    qDebug() << "Clicked on Tab " << index + 1;
    switch (index)
    {
    case 0:
        qDebug() << "Tab 1 clicked: Video Player tab";
        break;
    case 1:
        qDebug() << "Tab 2 clicked: Content for Tab 2";
        break;
    case 2:
        qDebug() << "Tab 3 clicked: Content for Tab 3";
        break;
    default:
        break;
    }
}
