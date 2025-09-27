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
#include <QMenu>
#include <QAction>

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

    // 设置自定义代理来处理绘制
    treeView->setItemDelegate(new VirtualItemDelegate(treeView));

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
        auto symbolicLink = obj["symbolicLink"].toString();
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
            item->setIcon(symbolicLink.isEmpty() ? QIcon(":/icons/folder.png") : QIcon(":/icons/folder_link.png"));
        } else {
            QString suffix = name.section('.', -1).toLower();

            QString iconPath = ":/icons/" + suffix + ".png";
            
            QIcon fileIcon = QFile::exists(iconPath) ? QIcon(iconPath) : (symbolicLink.isEmpty() ? QIcon(":/icons/file.png") : QIcon(":/icons/file_link.png"));

            item->setIcon(fileIcon);
        }

        item->setEditable(false);

        // 检查是否为隐藏文件或文件夹
        if (name.startsWith('.')) {
            item->setData(true, Qt::UserRole + 1); // 设置标记，表示该项是隐藏文件
        }

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

    return nullptr;
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

void RemoteFileExplorer::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu(this);

    QModelIndex index = treeView->indexAt(event->pos());
    if (!index.isValid()) return;

    QStandardItem *item = model->itemFromIndex(index);
    
    // 创建菜单项
    QAction *openAction = new QAction("打开", &contextMenu);
    connect(openAction, &QAction::triggered, this, [this, item]() {
        // 这里可以添加打开文件的逻辑
        qDebug() << "打开文件: " << item->text();
    });

    QAction *renameAction = new QAction("重命名", &contextMenu);
    connect(renameAction, &QAction::triggered, this, [this, item]() {
        // 这里可以添加重命名文件的逻辑
        qDebug() << "重命名文件: " << item->text();
    });

    QAction *deleteAction = new QAction("删除", &contextMenu);
    connect(deleteAction, &QAction::triggered, this, [this, item]() {
        // 这里可以添加删除文件的逻辑
        qDebug() << "删除文件: " << item->text();
    });

    // 将菜单项添加到右键菜单
    contextMenu.addAction(openAction);
    contextMenu.addAction(renameAction);
    contextMenu.addAction(deleteAction);

    // 显示右键菜单
    contextMenu.exec(event->globalPos());
}
