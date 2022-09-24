#pragma once

#include <memory>

#include <QObject>
#include <QAbstractSocket>
#include <QHostAddress>

#include "webserver_global.h"

class QTcpServer;

class WebserverClientConnection;
class Request;

class WEBSERVER_EXPORT AbstractWebserver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractWebserver)

public:
    explicit AbstractWebserver(QObject *parent = nullptr);
    ~AbstractWebserver() override;

    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    void close();

    bool isListening() const;

    void setMaxPendingConnections(int numConnections);
    int maxPendingConnections() const;

    quint16 serverPort() const;
    QHostAddress serverAddress() const;

    qintptr socketDescriptor() const;
    bool setSocketDescriptor(qintptr socketDescriptor);

    QAbstractSocket::SocketError serverError() const;
    QString errorString() const;

    void pauseAccepting();
    void resumeAccepting();

#ifndef QT_NO_NETWORKPROXY
    void setProxy(const QNetworkProxy &networkProxy);
    QNetworkProxy proxy() const;
#endif

protected:
    friend class WebserverClientConnection;
    virtual void requestReceived(WebserverClientConnection &client, const Request &request) = 0;

Q_SIGNALS:
    void acceptError(QAbstractSocket::SocketError socketError);

private slots:
    void newConnection();

private:
    const std::unique_ptr<QTcpServer> m_server;
};
