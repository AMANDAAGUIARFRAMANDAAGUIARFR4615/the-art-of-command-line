#pragma once

#include <QDateTime>

#define __PROJECT_FILE__ (__FILE__ + sizeof(PROJECT_SOURCE_DIR))

#define qDebugEx() qDebug().noquote() << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "]" << __PROJECT_FILE__ << __LINE__ << __FUNCTION__
#define qCriticalEx() qCritical().noquote() << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "]" << __PROJECT_FILE__ << __LINE__ << __FUNCTION__
