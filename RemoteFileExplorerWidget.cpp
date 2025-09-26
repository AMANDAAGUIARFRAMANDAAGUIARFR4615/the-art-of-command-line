#include "RemoteFileExplorerWidget.h"
#include <QVBoxLayout>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QMessageBox>
#include <QStyle>
#include <QApplication>

RemoteFileExplorerWidget::RemoteFileExplorerWidget(QWidget *parent)
    : QWidget(parent), manager(new QNetworkAccessManager(this))
{
    treeView = new QTreeView(this);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(treeView);
    setLayout(layout);
    setWindowTitle("远程文件系统视图");
    resize(800, 600);

    model = new QStandardItemModel();
    model->setHorizontalHeaderLabels({"文件夹名称"});
    treeView->setModel(model);

    connect(treeView, &QTreeView::expanded, this, &RemoteFileExplorerWidget::onDirectoryExpanded);
    getDirectoryList("/");
}

void RemoteFileExplorerWidget::getDirectoryList(const QString &path)
{
    if (loadedDirectories.contains(path)) return;

    QUrl url("http://192.168.0.112:8080/list?path=" + QUrl::toPercentEncoding(path));
    QNetworkRequest request(url);
    auto reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, path] { onReplyFinished(reply, path); });
}

void RemoteFileExplorerWidget::onReplyFinished(QNetworkReply *reply, const QString &path)
{
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, "错误", "无法连接到服务器: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    auto jsonDoc = QJsonDocument::fromJson(reply->readAll());
    if (!jsonDoc.isArray()) {
        QMessageBox::warning(this, "错误", "返回的数据格式错误");
        reply->deleteLater();
        return;
    }

    auto directories = jsonDoc.array();
    auto style = QApplication::style();
    auto folderIcon = style->standardIcon(QStyle::SP_DirIcon);
    auto fileIcon = style->standardIcon(QStyle::SP_FileIcon);

    auto rootItem = model->invisibleRootItem();

    for (const auto &value : directories) {
        auto obj = value.toObject();
        QString name = obj["name"].toString();
        QString type = obj["type"].toString();
        QString newPath = obj["path"].toString();

        auto item = new QStandardItem(name);
        item->setData(newPath, Qt::UserRole);
        item->setIcon(type == "folder" ? folderIcon : fileIcon);
        item->setEditable(type == "folder");
        if (type == "folder") item->setChild(0, nullptr);

        if (path == "/") rootItem->appendRow(item);
        else {
            auto parentItem = findItemByPath(path);
            if (parentItem) parentItem->appendRow(item);
        }

        loadedDirectories[path] = true;
    }

    reply->deleteLater();
}

QStandardItem* RemoteFileExplorerWidget::findItemByPath(const QString &path)
{
    for (int row = 0; row < model->invisibleRootItem()->rowCount(); ++row) {
        auto item = model->invisibleRootItem()->child(row);
        if (item->data(Qt::UserRole).toString() == path)
            return item;
    }
    return nullptr;
}

void RemoteFileExplorerWidget::onDirectoryExpanded(const QModelIndex &index)
{
    QString path = index.data(Qt::UserRole).toString();
    qDebug() << "展开目录路径: " << path;

    if (!path.isEmpty()) getDirectoryList(path);
    else qDebug() << "路径为空，检查是否正确设置路径。";
}
