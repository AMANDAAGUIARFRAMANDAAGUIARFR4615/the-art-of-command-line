#include "MainWindow.h"
#include "Logger.h"
#include "CenteredItemDelegate.h"

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
#include <QListWidgetItem>
#include <QStyle>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    auto central = new QWidget(this);
    auto mainLayout = new QHBoxLayout(central);  // 使用 QHBoxLayout 使左侧和右侧在一行内排列

    // 创建 QSplitter 用于分隔左右面板
    auto splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setStyleSheet("QSplitter::handle {"
                            "background-color: #B0B0B0;"
                            "width: 1px;"
                            "}");

    // 左侧导航栏
    auto sideBarWidget = new QWidget(this);
    sideBarWidget->setFixedWidth(70);
    sideBarWidget->setStyleSheet("background: transparent; border: none;");

    auto sideBarLayout = new QVBoxLayout(sideBarWidget);

    auto sideBarList = new QListWidget(sideBarWidget);

    // 设置列表项显示样式
    sideBarList->setViewMode(QListView::ListMode);
    sideBarList->setIconSize(QSize(36, 36));
    sideBarList->setSpacing(5);
    sideBarList->setItemDelegate(new CenteredItemDelegate(this));
    sideBarList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto style = sideBarWidget->style();

    // auto item1 = new QListWidgetItem(QIcon::fromTheme("user-home"), "");
    auto item1 = new QListWidgetItem(QIcon(style->standardIcon(QStyle::SP_DesktopIcon)), "");
    item1->setToolTip("提示1");
    auto item2 = new QListWidgetItem(QIcon(style->standardIcon(QStyle::SP_ComputerIcon)), "");
    item2->setToolTip("提示2");
    auto item3 = new QListWidgetItem(QIcon(style->standardIcon(QStyle::SP_BrowserReload)), "");
    item3->setToolTip("提示3");
    auto item4 = new QListWidgetItem(QIcon(style->standardIcon(QStyle::SP_FileIcon)), "");
    item4->setToolTip("提示4");

    sideBarList->addItem(item1);
    sideBarList->addItem(item2);
    sideBarList->addItem(item3);
    sideBarList->addItem(item4);

    sideBarLayout->addWidget(sideBarList);

    splitter->addWidget(sideBarWidget);

    // 右侧区域：Tab 和底部小布局
    auto rightWidget = new QWidget(this);
    auto rightLayout = new QVBoxLayout(rightWidget);

    // 上方大区域：Tab 标签页
    auto tabWidget = new QTabWidget(this);
    auto tab1 = new QWidget();
    auto tab2 = new QWidget();
    auto tab3 = new QWidget();

    tab1->setLayout(new QVBoxLayout());

    auto player = new VideoPlayer();
    // player->setUrl(QString("tcp://192.168.0.102:23145"));

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
    auto bottomWidget = new QWidget(this);
    auto gridLayout = new QGridLayout(bottomWidget);

    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 6; ++col)
        {
            auto frame = new QFrame(bottomWidget);
            frame->setFrameShape(QFrame::Box);
            frame->setLineWidth(2);

            auto frameLayout = new QVBoxLayout(frame);

            auto videoPlayer = new VideoPlayer(frame);
            // videoPlayer->setUrl(QString("tcp://192.168.0.102:23145")); // 需要时可单独设置不同的URL
            frameLayout->addWidget(videoPlayer);

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
    qDebugEx() << "Clicked on Tab " << index + 1;
    switch (index)
    {
    case 0:
        qDebugEx() << "Tab 1 clicked: Video Player tab";
        break;
    case 1:
        qDebugEx() << "Tab 2 clicked: Content for Tab 2";
        break;
    case 2:
        qDebugEx() << "Tab 3 clicked: Content for Tab 3";
        break;
    default:
        break;
    }
}
