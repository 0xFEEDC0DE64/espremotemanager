#include "abstractwebserver.h"

#include <QTcpServer>
#ifndef QT_NO_NETWORKPROXY
#include <QNetworkProxy>
#endif

#include "webserverclientconnection.h"

AbstractWebserver::AbstractWebserver(QObject *parent) :
    QObject{parent},
    m_server{std::make_unique<QTcpServer>(this)}
{
    connect(m_server.get(), &QTcpServer::newConnection, this, &AbstractWebserver::newConnection);
    connect(m_server.get(), &QTcpServer::acceptError, this, &AbstractWebserver::acceptError);
}

AbstractWebserver::~AbstractWebserver() = default;

bool AbstractWebserver::listen(const QHostAddress &address, quint16 port)
{
    return m_server->listen(address, port);
}

void AbstractWebserver::close()
{
    m_server->close();
}

bool AbstractWebserver::isListening() const
{
    return m_server->isListening();
}

void AbstractWebserver::setMaxPendingConnections(int numConnections)
{
    m_server->setMaxPendingConnections(numConnections);
}

int AbstractWebserver::maxPendingConnections() const
{
    return m_server->maxPendingConnections();
}

quint16 AbstractWebserver::serverPort() const
{
    return m_server->serverPort();
}

QHostAddress AbstractWebserver::serverAddress() const
{
    return m_server->serverAddress();
}

qintptr AbstractWebserver::socketDescriptor() const
{
    return m_server->socketDescriptor();
}

bool AbstractWebserver::setSocketDescriptor(qintptr socketDescriptor)
{
    return m_server->setSocketDescriptor(socketDescriptor);
}

QAbstractSocket::SocketError AbstractWebserver::serverError() const
{
    return m_server->serverError();
}

QString AbstractWebserver::errorString() const
{
    return m_server->errorString();
}

void AbstractWebserver::pauseAccepting()
{
    m_server->pauseAccepting();
}

void AbstractWebserver::resumeAccepting()
{
    m_server->resumeAccepting();
}

#ifndef QT_NO_NETWORKPROXY
void AbstractWebserver::setProxy(const QNetworkProxy &networkProxy)
{
    m_server->setProxy(networkProxy);
}

QNetworkProxy AbstractWebserver::proxy() const
{
    return m_server->proxy();
}
#endif

void AbstractWebserver::newConnection()
{
    while (const auto socket = m_server->nextPendingConnection())
        new WebserverClientConnection{*socket, *this, this};
}
