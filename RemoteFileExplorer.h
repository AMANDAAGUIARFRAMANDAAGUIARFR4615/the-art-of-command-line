#ifndef REMOTEFILEEXPLORER_H
#define REMOTEFILEEXPLORER_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMap>
#include <QTcpSocket>

class RemoteFileExplorer : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteFileExplorer(QTcpSocket* socket, QWidget *parent = nullptr);
    ~RemoteFileExplorer() = default;

protected:
    void fetchDirectoryContents(const QString &path);
    void updateDirectoryView(const QString &path, const QJsonArray &list);
    QStandardItem* findItemByPath(const QString &path);
    QStandardItem* findItemByPathRecursive(QStandardItem* parentItem, const QStringList &pathParts);
    void onDirectoryExpanded(const QModelIndex &index);
    void keyPressEvent(QKeyEvent *event) override;

    QTcpSocket* socket;
    QTreeView *treeView;
    QStandardItemModel *model;
};

#endif // REMOTEFILEEXPLORER_H
