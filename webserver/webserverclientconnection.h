#pragma once

#include <memory>
#include <QObject>
#include <QMap>

#include "webserver_global.h"
#include "webservercontainer.h"

class QTcpSocket;

class AbstractWebserver;

class WEBSERVER_EXPORT WebserverClientConnection : public QObject
{
    Q_OBJECT

public:
    explicit WebserverClientConnection(QTcpSocket &socket, AbstractWebserver &webserver, QObject *parent = nullptr);
    ~WebserverClientConnection() override;

    bool sendResponseHeaders(int status, const QByteArray &message,
                             const QMap<QByteArray, QByteArray> &responseHeaders);
    bool sendFullResponse(int status, const QByteArray &message,
                          QMap<QByteArray, QByteArray> responseHeaders, const QByteArray &response);

private slots:
    void readyRead();

private:
    bool writeLine(const QByteArray &buf);

    const std::unique_ptr<QTcpSocket> m_socket;
    AbstractWebserver &m_webserver;

    enum Status {
        RequestLine, RequestHeaders, Response
    };
    Status m_status{RequestLine};

    Request m_request;

    bool m_closeConnectionAfterResponse{};
};
