#ifndef REMOTEFILEEXPLORER_H
#define REMOTEFILEEXPLORER_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMap>

class RemoteFileExplorer : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteFileExplorer(QWidget *parent = nullptr);
    ~RemoteFileExplorer() = default;

private:
    void getDirectoryList(const QString &path);
    void onReplyFinished(QNetworkReply *reply, const QString &path);
    QStandardItem* findItemByPath(const QString &path);
    QStandardItem* findItemByPathRecursive(QStandardItem* parentItem, const QStringList &pathParts);
    void onDirectoryExpanded(const QModelIndex &index);

    QNetworkAccessManager *manager;
    QTreeView *treeView;
    QStandardItemModel *model;
};

#endif // REMOTEFILEEXPLORER_H
