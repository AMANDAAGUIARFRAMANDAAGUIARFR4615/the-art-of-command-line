#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <QNetworkInterface>
#include <QHostAddress>
#include <QDebug>
#include <QString>
#include <QList>

class NetworkUtils
{
public:
    // 获取本机内网IP地址
    static QString getLocalIP()
    {
        // 获取所有网络接口
        QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
        
        for (const QNetworkInterface &interface : interfaces) {
            // 排除虚拟网卡接口，比如 vEthernet(WSL) 等
            if (interface.humanReadableName().contains("vEthernet") ||
                !interface.flags().testFlag(QNetworkInterface::IsUp) ||
                interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
                continue; // 跳过虚拟网卡接口
            }

            QList<QNetworkAddressEntry> entries = interface.addressEntries();
            for (const QNetworkAddressEntry &entry : entries) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    // 返回第一个有效的 IPv4 地址
                    return entry.ip().toString();
                }
            }
        }
        
        return QString();
    }

    // 遍历同一子网的IP地址，IP范围从 .1 到 .254
    static QList<QHostAddress> getSubnetIPs()
    {
        QList<QHostAddress> ipList;

        QString localIP = getLocalIP();

        QHostAddress networkAddress(localIP);
        if (networkAddress.isNull()) {
            qWarning() << "无效的子网地址";
            return ipList;
        }

        quint32 networkIpv4 = networkAddress.toIPv4Address();

        // 遍历 IP 地址范围（1到254）
        for (int i = 1; i <= 254; ++i) {
            // 直接修改最后一个字节，生成有效的主机地址
            quint32 ipIpv4 = (networkIpv4 & 0xFFFFFF00) | i;

            QHostAddress ip(ipIpv4);

            if (ip.toString() != localIP)
                ipList.append(ip);
        }

        return ipList;
    }
};

#endif // NETWORKUTILS_H
