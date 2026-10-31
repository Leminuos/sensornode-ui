#ifndef NETWORK_INFO_H
#define NETWORK_INFO_H

#include <QString>

class NetworkInfo {
    public:
        static QString readMacAddress(const QString &iface);
};

#endif // NETWORK_INFO_H
