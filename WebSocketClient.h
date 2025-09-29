#pragma once

#include <QWebSocket>
#include <QObject>
#include <QUrl>

class WebSocketClient : public QObject {
    Q_OBJECT

public:
    WebSocketClient();
    void connectToServer(const QUrl &url);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);

private:
    QWebSocket m_webSocket;
};
