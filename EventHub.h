#ifndef EVENTHUB_H
#define EVENTHUB_H

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
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
    static void StartListening(const QString& eventName, std::function<void(QJsonObject, QTcpSocket*)> listener, int priority = 0)
    {
        listeners[eventName].emplace_back(listener, priority);

        auto& actions = listeners[eventName];
        std::sort(actions.begin(), actions.end(), [](const auto& a, const auto& b)
        {
            return a.second > b.second;
        });
    }

    static void StopListening(const QString& eventName, std::function<void(QJsonObject, QTcpSocket*)> listener = nullptr)
    {
        if (listeners.contains(eventName))
        {
            auto& actions = listeners[eventName];

            if (listener == nullptr)
            {
                actions.clear();
            }
            else
            {
                actions.erase(
                    std::remove_if(actions.begin(), actions.end(), 
                                   [&listener](const auto& pair) { return &pair.first == &listener; }),
                    actions.end());
            }
        }
    }

    static void TriggerEvent(const QString& eventName, const QJsonObject& data = QJsonObject(), QTcpSocket* socket = nullptr)
    {
        if (listeners.contains(eventName))
        {
            for (const auto& action : listeners[eventName])
            {
                action.first(data, socket);
            }
        }
    }

private:
    static QMap<QString, QList<std::pair<std::function<void(QJsonObject, QTcpSocket*)>, int>>> listeners;
};

QMap<QString, QList<std::pair<std::function<void(QJsonObject, QTcpSocket*)>, int>>> EventHub::listeners;

#endif // EVENTHUB_H
