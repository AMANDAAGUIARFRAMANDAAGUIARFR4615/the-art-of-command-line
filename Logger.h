#pragma once

#include <QDateTime>

#define __PROJECT_FILE__ (__FILE__ + sizeof(PROJECT_SOURCE_DIR))

// 日志前缀格式：LEVEL | hh:mm:ss | file:line@function |
#define __LOG_PREFIX__(level) \
    level << " | " << QDateTime::currentDateTime().toString("hh:mm:ss") \
          << " | " << QStringLiteral("%1:%2@%3") \
                        .arg(__PROJECT_FILE__) \
                        .arg(__LINE__) \
                        .arg(__FUNCTION__) \
          << " | "

#define qDebugEx()    qDebug().noquote()    << __LOG_PREFIX__("DEBUG")
#define qInfoEx()     qInfo().noquote()     << __LOG_PREFIX__("INFO ")
#define qWarningEx()  qWarning().noquote()  << __LOG_PREFIX__("WARN ")
#define qCriticalEx() qCritical().noquote() << __LOG_PREFIX__("ERROR")
