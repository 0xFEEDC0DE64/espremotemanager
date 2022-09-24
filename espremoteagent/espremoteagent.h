#pragma once

#include <vector>
#include <memory>

#include "abstractwebserver.h"

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
    explicit EspRemoteAgent(std::vector<SerialPortConfig> &&serialPortConfigs, QObject *parent = nullptr);
    ~EspRemoteAgent() override;

protected:
    void requestReceived(WebserverClientConnection &client, const Request &request) override;

private:
    void sendRootResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);
    void sendOpenResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);
    void sendCloseResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);
    void sendRebootResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);
    void sendSetDTRResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);
    void sendSetRTSResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query);

    std::vector<std::unique_ptr<EspRemotePort>> m_ports;
};
