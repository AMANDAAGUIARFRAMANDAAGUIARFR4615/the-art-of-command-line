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
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QDir>

RemoteFileExplorer::RemoteFileExplorer(QTcpSocket* socket, QWidget *parent) : socket(socket), QWidget(parent)
{
    treeView = new QTreeView(this);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(treeView);
    setLayout(layout);
    resize(800, 600);

    model = new QStandardItemModel();
    // model->setHorizontalHeaderLabels({"文件夹名称"});
    treeView->setModel(model);

    // 启用拖放功能
    treeView->setAcceptDrops(true);
    treeView->setDragEnabled(true);
    treeView->setDropIndicatorShown(true);

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

    QAction *deleteAction = new QAction("删除", &contextMenu);
    connect(deleteAction, &QAction::triggered, this, [this, item]() {
        qDebugEx() << "删除文件: " << item->text();
    });

    contextMenu.addAction(openAction);
    contextMenu.addAction(renameAction);
    contextMenu.addAction(deleteAction);

    contextMenu.exec(event->globalPos());
}

void RemoteFileExplorer::dragEnterEvent(QDragEnterEvent *event)
{
    qDebugEx() << "dragEnterEvent";

    const QMimeData *mimeData = event->mimeData();

    // 检查拖入的数据是否包含 URL（通常是文件路径）
    if (mimeData->hasUrls()) {
        // 遍历所有拖入的 URLs，并打印它们的本地路径
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl &url : urls) {
            QString filePath = url.toLocalFile();  // 获取本地文件路径
            qDebugEx() << "拖入的文件路径：" << filePath;  // 打印路径
        }
        event->acceptProposedAction();  // 允许拖放
    } else {
        // 打印没有 URL 的情况，并检查其他数据类型
        qDebugEx() << "拖入的数据不包含有效的 URL";

        // 检查是否包含文本
        if (mimeData->hasText()) {
            QString text = mimeData->text();
            qDebugEx() << "拖入的文本内容：" << text;
        }

        // 检查是否包含HTML
        if (mimeData->hasHtml()) {
            QString html = mimeData->html();
            qDebugEx() << "拖入的HTML内容：" << html;
        }

        // 检查是否包含图片
        if (mimeData->hasImage()) {
            QImage image = mimeData->imageData().value<QImage>();
            qDebugEx() << "拖入的图片：" << image.size();
        }

        // 检查是否包含其他数据（比如自定义数据）
        if (mimeData->hasFormat("application/x-my-custom-format")) {
            QByteArray customData = mimeData->data("application/x-my-custom-format");
            qDebugEx() << "拖入的自定义数据：" << customData;
        }

        event->ignore();  // 忽略不包含 URL 的拖拽
    }
}

void RemoteFileExplorer::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty()) {
            // 获取目标目录路径
            QModelIndex index = treeView->indexAt(event->pos());
            QString targetPath = index.data(Qt::UserRole).toString();

            for (const QUrl &url : urls) {
                QString filePath = url.toLocalFile();
                if (!filePath.isEmpty()) {
                    // 处理文件拷贝或移动
                    qDebugEx() << "将文件从" << filePath << "拖放到" << targetPath;
                    QFileInfo fileInfo(filePath);
                    QString targetFilePath = targetPath + "/" + fileInfo.fileName();

                    if (QFile::exists(targetFilePath)) {
                        QMessageBox::warning(this, "文件已存在", "目标文件已存在，请选择不同的文件名。");
                    } else {
                        QFile::copy(filePath, targetFilePath); // 复制文件到目标路径
                        fetchDirectoryContents(targetPath); // 更新目标路径下的文件列表
                    }
                }
            }
        }
    }
}
