#ifndef CENTEREDITEMDELEGATE_H
#define CENTEREDITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>

class CenteredItemDelegate : public QStyledItemDelegate {
public:
    explicit CenteredItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        auto icon = index.data(Qt::DecorationRole).value<QIcon>();
        if (!icon.isNull()) {
            auto iconSize = icon.actualSize(option.rect.size(), QIcon::Normal);
            auto iconRect = QRect(option.rect.center() - QPoint(iconSize.width() / 2, iconSize.height() / 2), iconSize);
            icon.paint(painter, iconRect);
        }
    }
};

#endif // CENTEREDITEMDELEGATE_H
