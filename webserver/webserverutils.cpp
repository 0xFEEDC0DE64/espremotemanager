#include "webserverutils.h"

QHostAddress parseHostAddress(const QString &str)
{
    if (str.isEmpty() || str == "Any")
        return QHostAddress::Any;
    else if (str == "AnyIPv6")
        return QHostAddress::AnyIPv6;
    else if (str == "AnyIPv4")
        return QHostAddress::AnyIPv4;
    else if (str == "LocalHost")
        return QHostAddress::LocalHost;
    else if (str == "LocalHostIPv6")
        return QHostAddress::LocalHostIPv6;
    else
        return QHostAddress{str};
}
