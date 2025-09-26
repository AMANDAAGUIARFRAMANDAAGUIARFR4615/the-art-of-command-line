#include "RemoteFileExplorer.h"
#include "Logger.h"
#include "EventHub.h"
#include "TcpServer.h"
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
#include <QKeyEvent>

RemoteFileExplorer::RemoteFileExplorer(QTcpSocket* socket, QWidget *parent) : socket(socket), QWidget(parent)
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

    connect(treeView, &QTreeView::expanded, this, &RemoteFileExplorer::onDirectoryExpanded);
    fetchDirectoryContents("/");

    EventHub::StartListening("fileList", [this, socket](QJsonObject data, QTcpSocket* s) {
        if (s != socket)
            return;

        auto path = data["path"].toString();
        auto list = data["list"].toArray();
        updateDirectoryView(path, list);
    });
}

void RemoteFileExplorer::fetchDirectoryContents(const QString &path)
{
    QJsonObject jsonObject;
    jsonObject["event"] = "fileList";
    jsonObject["data"] = path;
    TcpServer::sendData(socket, jsonObject);
}

void RemoteFileExplorer::updateDirectoryView(const QString &path, const QJsonArray &list)
{
    auto parentItem = findItemByPath(path);
    parentItem->removeRows(0, parentItem->rowCount());

    if (list.count() == 0) {
        qDebugEx() << path << "没有数据";
        return;
    }

    qDebugEx() << "updateDirectoryView" << path << list.count();

    QFileIconProvider iconProvider;

    for (const auto &value : list) {
        auto obj = value.toObject();
        auto name = obj["name"].toString();
        auto type = obj["type"].toString();
        auto myPath = path == '/' ? '/' + name : path + '/' + name;

        auto item = new QStandardItem(name);
        item->setData(myPath, Qt::UserRole);

        // NSFileTypeDirectory;
        // NSFileTypeRegular;
        // NSFileTypeSymbolicLink;
        // NSFileTypeSocket;
        // NSFileTypeCharacterSpecial;
        // NSFileTypeBlockSpecial;
        // NSFileTypeUnknown;

        auto isDirectory = type == "NSFileTypeDirectory" || type == "NSFileTypeSymbolicLink";

        if (isDirectory) {
            item->setIcon(iconProvider.icon(QFileIconProvider::Folder));
        } else {
            QFileInfo fileInfo(name);
            item->setIcon(iconProvider.icon(fileInfo));
        }

        item->setEditable(false);
        
        if (isDirectory) item->setChild(0, nullptr);

        parentItem->appendRow(item);
    }
}

QStandardItem* RemoteFileExplorer::findItemByPath(const QString &path)
{
    qDebugEx() << "findItemByPath" << path;

    // 分割路径，得到每一层的目录名
    QStringList pathParts = path.split('/', Qt::SkipEmptyParts);
    
    // 从根目录开始查找
    return findItemByPathRecursive(model->invisibleRootItem(), pathParts);
}

QStandardItem* RemoteFileExplorer::findItemByPathRecursive(QStandardItem* parentItem, const QStringList &pathParts)
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

void RemoteFileExplorer::onDirectoryExpanded(const QModelIndex &index)
{
    QString path = index.data(Qt::UserRole).toString();
    qDebug() << "展开目录路径: " << path;

    if (!path.isEmpty()) fetchDirectoryContents(path);
    else qDebug() << "路径为空，检查是否正确设置路径。";
}

void RemoteFileExplorer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        close();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}
