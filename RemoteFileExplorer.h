#ifndef REMOTEFILEEXPLORER_H
#define REMOTEFILEEXPLORER_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMap>
#include <QTcpSocket>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStatusBar>

class VirtualItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    VirtualItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();

        QStyleOptionViewItem modifiedOption = option;

        // 获取自定义数据标记，判断是否为隐藏文件
        bool isHidden = index.data(Qt::UserRole + 1).toBool();

        if (isHidden) {
            modifiedOption.palette.setColor(QPalette::Text, QColor(150, 150, 150));
            painter->setOpacity(0.5);
        }

        QStyledItemDelegate::paint(painter, modifiedOption, index);

        painter->restore();
    }
};

class RemoteFileExplorer : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteFileExplorer(QTcpSocket* socket, QWidget *parent = nullptr);
    ~RemoteFileExplorer() = default;

protected:
    void setStatusMessage(const QString &message);
    void fetchDirectoryContents(const QString &path);
    void fetchDirectoryContents(const QModelIndex &index);
    void updateDirectoryView(const QString &path, const QJsonArray &list);
    QStandardItem* findItemByPath(const QString &path);
    QStandardItem* findItemByPathRecursive(QStandardItem* parentItem, const QStringList &pathParts);
    void onDirectoryExpanded(const QModelIndex &index);
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    QTcpSocket* socket;
    QTreeView *treeView;
    QStandardItemModel *model;
    QStatusBar *statusBar;
};

#endif // REMOTEFILEEXPLORER_H
