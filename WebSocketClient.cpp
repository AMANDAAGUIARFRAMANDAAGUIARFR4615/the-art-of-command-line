#include "WebSocketClient.h"
#include "Logger.h"

WebSocketClient::WebSocketClient() {
    connect(&m_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
}

void WebSocketClient::connectToServer(const QUrl &url) {
    m_webSocket.open(url);
}

void WebSocketClient::onConnected() {
    qDebugT() << "WebSocket connected!";
    m_webSocket.sendTextMessage("Hello, WebSocket!");
}

void WebSocketClient::onDisconnected() {
    qDebugT() << "WebSocket disconnected!";
}

void WebSocketClient::onTextMessageReceived(const QString &message) {
    qDebugT() << "Message received: " << message;
}
