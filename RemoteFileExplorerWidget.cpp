#include "RemoteFileExplorerWidget.h"
#include "Logger.h"

#include <QVBoxLayout>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QMessageBox>
#include <QStyle>
#include <QApplication>
#include <QFileIconProvider>

RemoteFileExplorerWidget::RemoteFileExplorerWidget(QWidget *parent) : QWidget(parent), manager(new QNetworkAccessManager(this))
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

    auto parentItem = findItemByPath(path);
    parentItem->removeRows(0, parentItem->rowCount());

    auto directories = jsonDoc.array();
    if (directories.count() == 0) {
        qDebugEx() << path << "没有数据";
        return;
    }

    qDebugEx() << "onReplyFinished" << directories.count();

    QFileIconProvider iconProvider;

    for (const auto &value : directories) {
        auto obj = value.toObject();
        auto name = obj["name"].toString();
        auto type = obj["type"].toString();
        auto myPath = path == '/' ? '/' + name : path + '/' + name;

        auto item = new QStandardItem(name);
        item->setData(myPath, Qt::UserRole);

        if (type == "folder") {
            item->setIcon(iconProvider.icon(QFileIconProvider::Folder));
        } else {
            QFileInfo fileInfo(name);
            item->setIcon(iconProvider.icon(fileInfo));
        }

        item->setEditable(type == "folder");
        if (type == "folder") item->setChild(0, nullptr);

        parentItem->appendRow(item);
    }

    reply->deleteLater();
}

QStandardItem* RemoteFileExplorerWidget::findItemByPath(const QString &path)
{
    qDebugEx() << "findItemByPath" << path;

    // 分割路径，得到每一层的目录名
    QStringList pathParts = path.split('/', Qt::SkipEmptyParts);
    
    // 从根目录开始查找
    return findItemByPathRecursive(model->invisibleRootItem(), pathParts);
}

QStandardItem* RemoteFileExplorerWidget::findItemByPathRecursive(QStandardItem* parentItem, const QStringList &pathParts)
{
    if (pathParts.isEmpty()) return parentItem;

    auto currentPart = pathParts.first();
    auto remainingPath = pathParts.mid(1).join('/');

    for (int row = 0; row < parentItem->rowCount(); ++row) {
        auto item = parentItem->child(row);

        if (currentPart == item->text())
            return findItemByPathRecursive(item, pathParts.mid(1));
    }

    return nullptr;  // 如果没有找到对应项
}

void RemoteFileExplorerWidget::onDirectoryExpanded(const QModelIndex &index)
{
    QString path = index.data(Qt::UserRole).toString();
    qDebug() << "展开目录路径: " << path;

    if (!path.isEmpty()) getDirectoryList(path);
    else qDebug() << "路径为空，检查是否正确设置路径。";
}
