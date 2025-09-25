#include "TestWindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>

TestWindow::TestWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("测试窗口");
    resize(400, 300);

    auto *layout = new QVBoxLayout(this);

    auto *btn1 = new QPushButton("按钮 A", this);
    auto *btn2 = new QPushButton("按钮 B", this);
    auto *btn3 = new QPushButton("按钮 C", this);

    layout->addWidget(btn1);
    layout->addWidget(btn2);
    layout->addWidget(btn3);

    connect(btn1, &QPushButton::clicked, this, [] {
        qDebug() << "按钮 A 被点击";
    });
    connect(btn2, &QPushButton::clicked, this, [] {
        qDebug() << "按钮 B 被点击";
    });
    connect(btn3, &QPushButton::clicked, this, [] {
        qDebug() << "按钮 C 被点击";
    });
}
