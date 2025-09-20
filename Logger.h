#pragma once

#include <QDateTime>

#define qDebugEx() qDebug().noquote() << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "]" << __FILE__ << __LINE__ << __FUNCTION__
#define qCriticalEx() qCritical().noquote() << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "]" << __FILE__ << __LINE__ << __FUNCTION__
