#pragma once

#include <vector>
#include <memory>

#include "abstractwebserver.h"

class QWebSocketServer;
class SerialPortConfig;
class WebserverClientConnection;
class Request;
class EspRemotePort;
class QUrl;
class QUrlQuery;

class EspRemoteAgent : public AbstractWebserver
{
    Q_OBJECT

public:
    explicit EspRemoteAgent(QWebSocketServer &websocketServer, std::vector<SerialPortConfig> &&serialPortConfigs, QObject *parent = nullptr);
    ~EspRemoteAgent() override;

protected:
    void requestReceived(WebserverClientConnection &client, const Request &request) override;

private slots:
    void newWebsocketConnect();

private:
    void sendRootResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);
    void sendOpenResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);
    void sendCloseResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);
    void sendRebootResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);
    void sendSetDTRResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);
    void sendSetRTSResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);

    QWebSocketServer &m_websocketServer;
    std::vector<std::unique_ptr<EspRemotePort>> m_ports;
};
