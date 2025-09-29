#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>

class ToastWidget : public QWidget {
public:
    explicit ToastWidget(const QString &message, QWidget *parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
        QLabel *label = new QLabel(message, this);
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(label);
        setLayout(layout);
        this->show();
        
        QTimer::singleShot(2000, this, &ToastWidget::deleteLater);
    }
};
