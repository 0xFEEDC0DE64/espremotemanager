#pragma once

#include <QByteArray>
#include <QMap>

#include "webserver_global.h"

struct WEBSERVER_EXPORT Request
{
    QByteArray method;
    QByteArray path;
    QByteArray protocol;
    QMap<QByteArray, QByteArray> headers;

    void clear()
    {
        method.clear();
        path.clear();
        protocol.clear();
        headers.clear();
    }
};
