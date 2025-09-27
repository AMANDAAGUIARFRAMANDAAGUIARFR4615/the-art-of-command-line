#ifndef EVENTHUB_H
#define EVENTHUB_H

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonValue>
#include <QMap>
#include <QList>
#include <functional>
#include <QString>
#include <memory>
#include <algorithm>
#include <QTcpSocket>

class EventHub
{
public:
    static void StartListening(const QString& eventName, std::function<void(QJsonValue, QTcpSocket*)> listener, int priority = 0);
    static void StopListening(const QString& eventName, std::function<void(QJsonValue, QTcpSocket*)> listener = nullptr);
    static void TriggerEvent(const QString& eventName, const QJsonValue& data = QJsonValue(), QTcpSocket* socket = nullptr);

private:
    static QMap<QString, QList<std::pair<std::function<void(QJsonValue, QTcpSocket*)>, int>>> listeners;
};

#endif // EVENTHUB_H
