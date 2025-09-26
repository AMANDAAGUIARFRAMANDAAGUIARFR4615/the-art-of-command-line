#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QJsonObject>
#include <QString>
#include <QDebug>

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
          version(json["version"].toString()) {}

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
    const QString version;

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

extern DeviceInfo deviceInfo;

#endif // DEVICEINFO_H
