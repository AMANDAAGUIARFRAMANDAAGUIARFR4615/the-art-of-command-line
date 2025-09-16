#include "MainWindow.h"
#include "TcpClient.h"
#include "ToastWidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();

    int x = (screenGeometry.width() - w.width()) / 2;
    int y = (screenGeometry.height() - w.height()) / 2;
    w.move(x, y);

    w.show();

    auto onConnected = []() {
        qDebug() << "与服务器连接成功！";
    };

    auto onDataReceived = [](const QByteArray &data) {
        qDebug() << "收到数据: " << data;
    };

    auto onDisconnected = []() {
        qDebug() << "与服务器断开连接";
    };

    auto onError = [&w](QAbstractSocket::SocketError error) {
        QString errorMessage;

        switch (error) {
            case QAbstractSocket::HostNotFoundError:
                errorMessage = "无法找到主机，检查IP地址是否正确。";
                break;
            case QAbstractSocket::ConnectionRefusedError:
                errorMessage = "连接被拒绝，目标主机未响应。";
                break;
            case QAbstractSocket::RemoteHostClosedError:
                errorMessage = "远程主机关闭了连接。";
                break;
            case QAbstractSocket::SocketTimeoutError:
                errorMessage = "连接超时，请检查网络。";
                break;
            default:
                errorMessage = "发生未知错误，错误代码：" + QString::number(error);
                break;
        }

        new ToastWidget(errorMessage);
    };

    TcpClient client(onConnected, onDataReceived, onDisconnected, onError);
    client.connectToServer("127.0.0.1", 3000);
    QJsonObject jsonObject;
    jsonObject["key1"] = "value1";
    jsonObject["key2"] = "value2";
    client.sendData(jsonObject);

    return a.exec();
}
