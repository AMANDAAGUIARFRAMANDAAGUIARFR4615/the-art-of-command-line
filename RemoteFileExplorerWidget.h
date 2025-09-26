#ifndef REMOTEFILEEXPLORERWIDGET_H
#define REMOTEFILEEXPLORERWIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMap>

class RemoteFileExplorerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteFileExplorerWidget(QWidget *parent = nullptr);
    ~RemoteFileExplorerWidget() = default;

private:
    void getDirectoryList(const QString &path);
    void onReplyFinished(QNetworkReply *reply, const QString &path);
    QStandardItem* findItemByPath(const QString &path);
    void onDirectoryExpanded(const QModelIndex &index);

    QNetworkAccessManager *manager;
    QTreeView *treeView;
    QStandardItemModel *model;
    QMap<QString, bool> loadedDirectories;
};

#endif // REMOTEFILEEXPLORERWIDGET_H
