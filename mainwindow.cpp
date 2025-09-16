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
#include <QSplitter>
#include <QListWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);  // 使用 QHBoxLayout 使左侧和右侧在一行内排列

    // 创建 QSplitter 用于分隔左右面板
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // 左侧导航栏
    QWidget *sideBarWidget = new QWidget(this);
    QVBoxLayout *sideBarLayout = new QVBoxLayout(sideBarWidget);

    QListWidget *sideBarList = new QListWidget(sideBarWidget);
    sideBarList->addItem("常看的号");
    sideBarList->addItem("CSDN");
    sideBarList->addItem("华为任职");
    sideBarList->addItem("人民日报");

    sideBarLayout->addWidget(sideBarList);
    sideBarWidget->setFixedWidth(100);

    splitter->addWidget(sideBarWidget);  // 将左侧导航栏添加到 splitter

    // 右侧区域：Tab 和底部小布局
    QWidget *rightWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);

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

    // 将 Tab 和底部布局添加到右侧区域
    rightLayout->addWidget(tabWidget, 2);
    rightLayout->addWidget(bottomWidget, 1);

    splitter->addWidget(rightWidget);  // 将右侧内容区域添加到 splitter

    mainLayout->addWidget(splitter);  // 将整个 splitter 放入主布局

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
