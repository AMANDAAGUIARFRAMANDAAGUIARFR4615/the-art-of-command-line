#pragma once

#include <QDateTime>

#define qDebugT() qDebug().noquote() << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "]"
#define qCriticalT() qCritical().noquote() << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "]"
