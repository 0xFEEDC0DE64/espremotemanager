#pragma once

#include <QString>
#include <QUrl>

struct SerialPortConfig
{
    QString port;
    int baudrate;
    QUrl url;
};
