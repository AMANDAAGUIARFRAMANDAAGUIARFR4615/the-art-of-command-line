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
#include <QInputDialog>

RemoteFileExplorer::RemoteFileExplorer(QTcpSocket* socket, QWidget *parent) : socket(socket), QWidget(parent)
{
    setAcceptDrops(true);

    treeView = new QTreeView(this);
    QFont font = treeView->font();
    font.setPointSize(12);
    treeView->setFont(font);
    treeView->setIconSize(QSize(24, 24)); 

    auto layout = new QVBoxLayout(this);
    layout->addWidget(treeView);

    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);

    setLayout(layout);

    model = new QStandardItemModel();
    model->setHorizontalHeaderLabels({"名称", "修改时间", "大小"});
    treeView->setModel(model);

    treeView->setItemDelegate(new VirtualItemDelegate(treeView));

    connect(treeView, &QTreeView::expanded, this, &RemoteFileExplorer::onDirectoryExpanded);
    fetchDirectoryContents("/");

    EventHub::StartListening("fileList", [this](QJsonValue data, QTcpSocket* socket) {
        if (this->socket != socket)
            return;

        auto path = data["path"].toString();
        auto list = data["list"].toArray();
        updateDirectoryView(path, list);
    });

    setStatusMessage("就绪");
}

void RemoteFileExplorer::setStatusMessage(const QString &message)
{
    auto timestamp = QTime::currentTime().toString("HH:mm:ss");
    statusBar->showMessage("[" + timestamp + "] " + message);
}

void RemoteFileExplorer::fetchDirectoryContents(const QString &path)
{
    qDebugEx() << "fetchDirectoryContents" << path;

    QJsonObject jsonObject;
    jsonObject["event"] = "fileList";
    jsonObject["data"] = path;
    TcpServer::sendData(socket, jsonObject);
}

void RemoteFileExplorer::fetchDirectoryContents(const QModelIndex &index)
{
    bool isDir = index.data(Qt::UserRole + 2).toBool();
    QString targetPath = (isDir ? index : index.parent()).data(Qt::UserRole).toString();
    fetchDirectoryContents(targetPath);
}

void RemoteFileExplorer::updateDirectoryView(const QString &path, const QJsonArray &list)
{
    auto parentItem = findItemByPath(path);
    if (!parentItem) {
        setStatusMessage("目录加载失败: " + path);
        return;
    }

    parentItem->removeRows(0, parentItem->rowCount());

    if (list.count() == 0) {
        setStatusMessage("目录为空: " + path);
        return;
    }

    QFileIconProvider iconProvider;

    for (const auto &value : list) {
        auto obj = value.toObject();
        auto name = obj["name"].toString();
        auto type = obj["type"].toString();
        auto symbolicLink = obj["symbolicLink"].toString();
        auto myPath = path == '/' ? '/' + name : path + '/' + name;
        auto date = obj["date"].toString();
        auto size = obj["size"].toInt();

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

        item->setData(isDirectory, Qt::UserRole + 2); // 隐藏是否文件夹

        if (isDirectory) item->setChild(0, nullptr);

        QStandardItem* dateItem = new QStandardItem(date);
        QStandardItem* sizeItem = new QStandardItem(Tools::formatByteSize(size));

        parentItem->appendRow({item, dateItem, sizeItem});
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
    setStatusMessage("展开目录: " + path);
    fetchDirectoryContents(path);
}

void RemoteFileExplorer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(event);
}

void RemoteFileExplorer::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu(this);

    QModelIndex index = treeView->selectionModel()->currentIndex();
    if (!index.isValid()) {
        qDebugEx() << "没有选中任何项，无法获取文件项";
        return;
    }

    auto targetPath = index.data(Qt::UserRole).toString();

    // QAction *openAction = new QAction("打开", &contextMenu);
    // contextMenu.addAction(openAction);
    // connect(openAction, &QAction::triggered, this, [this, &targetPath, &index]() {
    //     // qDebugEx() << "打开文件: " << item->text();
    // });

    QAction *renameAction = new QAction("重命名", &contextMenu);
    contextMenu.addAction(renameAction);
    connect(renameAction, &QAction::triggered, this, [this, &targetPath, &index]() {
        bool ok;
        auto name = QInputDialog::getText(this, "重命名", "请输入名称:", QLineEdit::Normal, "", &ok);
        
        if (!ok || name.isEmpty())
            return;

        setStatusMessage("重命名: " + name);

        QJsonObject dataObject;
        dataObject["atPath"] = targetPath;
        dataObject["toPath"] = name;

        QJsonObject jsonObject;
        jsonObject["event"] = "renameItem";
        jsonObject["data"] = dataObject;

        TcpServer::sendData(socket, jsonObject);
        fetchDirectoryContents(index.parent());
    });

    QAction *createAction = new QAction("新建文件夹", &contextMenu);
    contextMenu.addAction(createAction);
    connect(createAction, &QAction::triggered, this, [this, &targetPath, &index]() {
        bool ok;
        auto name = QInputDialog::getText(this, "新建文件夹", "请输入名称:", QLineEdit::Normal, "", &ok);
        
        if (!ok || name.isEmpty())
            return;

        setStatusMessage("新建文件夹: " + name);
            
        QJsonObject jsonObject;
        jsonObject["event"] = "createDirectory";
        jsonObject["data"] = targetPath + "/" + name;

        TcpServer::sendData(socket, jsonObject);
        fetchDirectoryContents(index);
    });

    QAction *deleteAction = new QAction("删除", &contextMenu);
    contextMenu.addAction(deleteAction);
    connect(deleteAction, &QAction::triggered, this, [this, &targetPath, &index]() {
        auto reply = QMessageBox::question(this, "确认删除", "你确定要删除【" + QFileInfo(targetPath).fileName() + "】吗？", 
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (reply != QMessageBox::Yes)
            return;

        setStatusMessage("删除: " + targetPath);
        
        QJsonObject jsonObject;
        jsonObject["event"] = "removeItem";
        jsonObject["data"] = targetPath;

        TcpServer::sendData(socket, jsonObject);
        fetchDirectoryContents(index.parent());
    });

    if (targetPath.endsWith(".zip") || targetPath.endsWith(".rar")) {
        QAction *extractAction = new QAction("解压", &contextMenu);
        contextMenu.addAction(extractAction);
        connect(extractAction, &QAction::triggered, this, [this, &targetPath, &index]() {
            setStatusMessage("解压: " + targetPath);

            QJsonObject jsonObject;
            jsonObject["event"] = "extractArchive";
            jsonObject["data"] = targetPath;

            TcpServer::sendData(socket, jsonObject);
            fetchDirectoryContents(index.parent());
        });
    }

    contextMenu.exec(event->globalPos());
}

void RemoteFileExplorer::dragEnterEvent(QDragEnterEvent *event)
{
    qDebugEx() << "dragEnterEvent";

    if (!event->mimeData()->hasUrls()) 
        event->ignore();

    auto urls = event->mimeData()->urls();

    for (const QUrl &url : urls) {
        if (QFileInfo(url.toLocalFile()).isDir()) {
            event->ignore();
            return;
        }
    }
    
    event->accept();
}

void RemoteFileExplorer::dropEvent(QDropEvent *event)
{
    auto urls = event->mimeData()->urls();

    QPoint globalPos = mapToGlobal(event->pos());
    QPoint localPos = treeView->viewport()->mapFromGlobal(globalPos);
    
    QModelIndex index = treeView->indexAt(localPos);
    QString targetPath = index.data(Qt::UserRole).toString();
    bool isDir = index.data(Qt::UserRole + 2).toBool();

    for (const QUrl &url : urls) {
        auto id = QUuid::createUuid().toString();
        auto type = 2; // 收是1，发是2
        auto path = url.toLocalFile();
        auto size = Tools::getFileSize(path);

        qDebugEx() << "将文件从" << path << "拖放到" << targetPath;

        auto dir = isDir ? targetPath : targetPath.left(targetPath.lastIndexOf('/'));
        auto transfer = new FileTransfer(type, path, size);

        QJsonObject dataObject;
        dataObject["id"] = id;
        dataObject["type"] = type;
        dataObject["port"] = transfer->serverPort();
        dataObject["path"] = dir + QString("/") + QFileInfo(path).fileName();
        dataObject["size"] = size;

        QJsonObject jsonObject;
        jsonObject["event"] = "transferFile";
        jsonObject["data"] = dataObject;

        TcpServer::sendData(socket, jsonObject);
    }

    fetchDirectoryContents(index);
    event->accept();
}
