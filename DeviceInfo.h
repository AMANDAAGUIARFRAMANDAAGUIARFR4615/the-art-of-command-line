#include <QJsonObject>
#include <QString>

class DeviceInfo {
public:
    DeviceInfo()
        : jbType(0), orientation(0), screenHeight(0), screenWidth(0) {}

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
};

QDebug operator<<(QDebug dbg, const DeviceInfo &deviceInfo);

extern DeviceInfo deviceInfo;
