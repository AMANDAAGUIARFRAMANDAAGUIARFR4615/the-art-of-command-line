#include "RemoteFileExplorer.h"
#include "Logger.h"
#include "EventHub.h"
#include "TcpServer.h"
#include "Tools.h"
#include "FileTransfer.h"
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
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QDir>

RemoteFileExplorer::RemoteFileExplorer(QTcpSocket* socket, QWidget *parent) : socket(socket), QWidget(parent)
{
    setAcceptDrops(true);

    treeView = new QTreeView(this);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(treeView);
    setLayout(layout);
    resize(800, 600);

    model = new QStandardItemModel();
    // model->setHorizontalHeaderLabels({"文件夹名称"});
    treeView->setModel(model);

    // 设置自定义代理来处理绘制
    treeView->setItemDelegate(new VirtualItemDelegate(treeView));

    connect(treeView, &QTreeView::expanded, this, &RemoteFileExplorer::onDirectoryExpanded);
    fetchDirectoryContents("/");

    EventHub::StartListening("fileList", [this, socket](QJsonValue data, QTcpSocket* s) {
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

        if (name.startsWith('.')) {
            item->setData(true, Qt::UserRole + 1); // 隐藏文件标记
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
    qDebugEx() << "展开目录路径: " << path;

    if (!path.isEmpty()) fetchDirectoryContents(path);
    else qDebugEx() << "路径为空，检查是否正确设置路径。";
}

void RemoteFileExplorer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void RemoteFileExplorer::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu(this);

    QModelIndex index = treeView->selectionModel()->currentIndex();
    if (!index.isValid()) {
        qDebugEx() << "没有选中任何项，无法获取文件项";
        return;
    }

    QStandardItem *item = model->itemFromIndex(index);
    if (!item) {
        qDebugEx() << "无法从索引获取标准项";
        return;
    }

    QAction *openAction = new QAction("打开", &contextMenu);
    connect(openAction, &QAction::triggered, this, [this, item]() {
        qDebugEx() << "打开文件: " << item->text();
    });

    QAction *renameAction = new QAction("重命名", &contextMenu);
    connect(renameAction, &QAction::triggered, this, [this, item]() {
        qDebugEx() << "重命名文件: " << item->text();
    });

    QAction *newFileAction = new QAction("新建文件", &contextMenu);
    connect(newFileAction, &QAction::triggered, this, [this, item]() {
        qDebugEx() << "文件文件: " << item->text();
    });

    QAction *newFolderAction = new QAction("新建文件夹", &contextMenu);
    connect(newFolderAction, &QAction::triggered, this, [this, item]() {
        qDebugEx() << "新建文件夹: " << item->text();
    });

    QAction *deleteAction = new QAction("删除", &contextMenu);
    connect(deleteAction, &QAction::triggered, this, [this, item]() {
        qDebugEx() << "删除文件: " << item->text();
    });

    contextMenu.addAction(openAction);
    contextMenu.addAction(renameAction);
    contextMenu.addAction(newFileAction);
    contextMenu.addAction(newFolderAction);
    contextMenu.addAction(deleteAction);

    contextMenu.exec(event->globalPos());
}

void RemoteFileExplorer::dragEnterEvent(QDragEnterEvent *event)
{
    qDebugEx() << "dragEnterEvent";

    if (event->mimeData()->hasUrls()) 
        event->accept();
    else
        event->ignore();
}

void RemoteFileExplorer::dropEvent(QDropEvent *event)
{
    auto urls = event->mimeData()->urls();

    QPoint globalPos = mapToGlobal(event->pos());
    QPoint localPos = treeView->viewport()->mapFromGlobal(globalPos);
    
    QModelIndex index = treeView->indexAt(localPos);
    QString targetPath = index.data(Qt::UserRole).toString();

    for (const QUrl &url : urls) {
        auto id = QUuid::createUuid().toString();
        auto type = 2; // 收是1，发是2
        auto path = url.toLocalFile();
        auto size = Tools::getFileSize(path);

        qDebugEx() << "将文件从" << path << "拖放到" << targetPath;

        QFileInfo fileInfo(path);

        auto dir = fileInfo.isFile() ? fileInfo.absoluteDir().path() : fileInfo.absolutePath();
        
        auto transfer = new FileTransfer(type, path, size);

        QJsonObject dataObject;
        dataObject["id"] = id;
        dataObject["type"] = type;
        dataObject["port"] = transfer->serverPort();
        dataObject["path"] = dir + QFileInfo(targetPath).fileName();
        dataObject["size"] = size;

        QJsonObject jsonObject;
        jsonObject["event"] = "transferFile";
        jsonObject["data"] = dataObject;
    }

    event->accept();
}
