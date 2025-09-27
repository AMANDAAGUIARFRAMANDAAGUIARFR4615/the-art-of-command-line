#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QJsonObject>
#include <QString>
#include <QGuiApplication>
#include <QScreen>

class DeviceInfo {
public:
    DeviceInfo(const QJsonObject &json)
        : deviceId(json["deviceId"].toString()),
          deviceName(json["deviceName"].toString()),
          inuse(json["inuse"].toString()),
          jbType(json["jbType"].toInt()),
          localIp(json["localIp"].toString()),
          orientation(json["orientation"].toInt()),
          platform(json["platform"].toString()),
          screenHeight(json["screenHeight"].toInt()),
          screenWidth(json["screenWidth"].toInt()),
          lockedStatus(json["lockedStatus"].toBool()),
          version(json["version"].toString()) {

        auto screenSize = QGuiApplication::primaryScreen()->size();
        auto maxWidth = screenSize.width() * 0.8;
        auto maxHeight = screenSize.height() * 0.8;

        if (screenWidth > maxWidth || screenHeight > maxHeight) {
            // 选择最小的缩放比例，保证宽高都不会超过屏幕
            scaleFactor = std::min(maxWidth / screenWidth, maxHeight / screenHeight);
        }
    }

    QString toString() const {
        return QString("DeviceInfo(deviceId: %1, deviceName: %2, inuse: %3, jbType: %4, "
                       "localIp: %5, orientation: %6, platform: %7, screenHeight: %8, "
                       "screenWidth: %9, version: %10)")
               .arg(deviceId)
               .arg(deviceName)
               .arg(inuse)
               .arg(jbType)
               .arg(localIp)
               .arg(orientation)
               .arg(platform)
               .arg(screenHeight)
               .arg(screenWidth)
               .arg(version);
    }

    const QString deviceId;
    const QString deviceName;
    const QString inuse;
    const int jbType;
    const QString localIp;
    const int orientation;
    const QString platform;
    const int screenHeight;
    const int screenWidth;
    const bool lockedStatus;
    const QString version;

    float scaleFactor = 1;

    friend QDebug operator<<(QDebug dbg, const DeviceInfo &deviceInfo) {
        dbg.nospace() << "DeviceInfo(deviceId: " << deviceInfo.deviceId
                      << ", deviceName: " << deviceInfo.deviceName
                      << ", inuse: " << deviceInfo.inuse
                      << ", jbType: " << deviceInfo.jbType
                      << ", localIp: " << deviceInfo.localIp
                      << ", orientation: " << deviceInfo.orientation
                      << ", platform: " << deviceInfo.platform
                      << ", screenHeight: " << deviceInfo.screenHeight
                      << ", screenWidth: " << deviceInfo.screenWidth
                      << ", version: " << deviceInfo.version << ")";
        return dbg;
    }
};

#endif // DEVICEINFO_H
