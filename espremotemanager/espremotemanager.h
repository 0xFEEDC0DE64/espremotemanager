#pragma once

#include <vector>

#include "abstractwebserver.h"

class QWebSocketServer;
class EspRemoteClient;
class QUrl;
class QUrlQuery;

class EspRemoteManager : public AbstractWebserver
{
public:
    explicit EspRemoteManager(QWebSocketServer &websocketServer, QObject *parent = nullptr);
    ~EspRemoteManager() override;

protected:
    void requestReceived(WebserverClientConnection &client, const Request &request) override;

private slots:
    void newConnection();
    void clientDestroyed(QObject *object);

private:
    void sendRootResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);
    void sendRebootResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);

    QWebSocketServer &m_websocketServer;

    std::vector<EspRemoteClient*> m_clients;
};
