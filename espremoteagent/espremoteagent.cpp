#include "espremoteagent.h"

#include <QDebug>
#include <QUrl>
#include <QUrlQuery>

#include "webservercontainer.h"
#include "webserverclientconnection.h"
#include "espremoteagentcontainers.h"
#include "espremoteport.h"

EspRemoteAgent::EspRemoteAgent(std::vector<SerialPortConfig> &&serialPortConfigs, QObject *parent) :
    AbstractWebserver{parent}
{
    m_ports.reserve(serialPortConfigs.size());

    for (auto &config : serialPortConfigs)
        m_ports.emplace_back(std::make_unique<EspRemotePort>(std::move(config), this));
}

EspRemoteAgent::~EspRemoteAgent() = default;

void EspRemoteAgent::requestReceived(WebserverClientConnection &client, const Request &request)
{
    const QUrl url{request.path};
    const QUrlQuery query{url};

    if (url.path() == "/")
    {
        sendRootResponse(client, url, query);
    }
    else if (url.path() == "/open")
    {
        sendOpenResponse(client, url, query);
    }
    else if (url.path() == "/close")
    {
        sendCloseResponse(client, url, query);
    }
    else if (url.path() == "/reboot")
    {
        sendRebootResponse(client, url, query);
    }
    else if (url.path() == "/setDTR")
    {
        sendSetDTRResponse(client, url, query);
    }
    else if (url.path() == "/setRTS")
    {
        sendSetRTSResponse(client, url, query);
    }
    else
        if (!client.sendFullResponse(404, "Not Found", {{"Content-Type", "text/plain"}}, "The requested path \"" + request.path + "\" was not found."))
            qWarning() << "sending response failed";
}

void EspRemoteAgent::sendRootResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query)
{
    QString content =
        "<html>"
            "<head>"
                "<title>ESP Remote Agent</title>"
            "</head>"
            "<body>"
                "<table border=\"1\">"
                    "<thead>"
                        "<tr>"
                            "<th>ID</th>"
                            "<th>Port</th>"
                            "<th>Status</th>"
                            "<th>Message</th>"
                            "<th>Actions</th>"
                            "<th>Log output</th>"
                        "</tr>"
                    "</thead>"
                    "<tbody>";

    std::size_t i{};
    for (const auto &port : m_ports)
    {
        const auto currentId = i++;
        content += QStringLiteral("<tr>"
                                    "<td>%0</td>"
                                    "<td>%1</td>"
                                    "<td>%2</td>"
                                    "<td>%3</td>"
                                    "<td>"
                                      "<a href=\"open?id=%0\">Open</a> "
                                      "<a href=\"close?id=%0\">Close</a><br />"
                                      "<a href=\"reboot?id=%0\">Reboot</a><br />"
                                      "DTR %4 <a href=\"setDTR?id=%0&set=%5\">Toggle</a><br />"
                                      "RTS %6 <a href=\"setRTS?id=%0&set=%7\">Toggle</a>"
                                    "</td>"
                                    "<td><pre>%8</pre></td>"
                                  "</tr>")
                .arg(currentId)
                .arg(port->port())
                .arg(port->status())
                .arg(port->message())
                .arg(port->isDataTerminalReady() ? "On" : "Off")
                .arg(port->isDataTerminalReady() ? "false" : "true")
                .arg(port->isRequestToSend() ? "On" : "Off")
                .arg(port->isRequestToSend() ? "false" : "true")
                .arg(port->logOutput());
    }

    content +=      "</tbody>"
                "</table>"
            "</body>"
        "</html>";

    if (!client.sendFullResponse(200, "Ok", {{"Content-Type", "text/html"}}, content.toUtf8()))
        qWarning() << "sending response failed";
}

void EspRemoteAgent::sendOpenResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query)
{
    if (!query.hasQueryItem("id"))
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "id missing"))
            qWarning() << "sending response failed";
        return;
    }
    const auto idStr = query.queryItemValue("id");
    bool ok{};
    const auto id = idStr.toInt(&ok);
    if (!ok)
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "could not parse id"))
            qWarning() << "sending response failed";
        return;
    }
    if (id < 0 || id >= m_ports.size())
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "id out of range"))
            qWarning() << "sending response failed";
        return;
    }

    if ((*std::next(std::begin(m_ports), id))->tryOpen())
    {
        if (!client.sendFullResponse(200, "Ok", {{"Content-Type", "text/plain"}}, "Port opened successfully!"))
            qWarning() << "sending response failed";
        return;
    }
    else
    {
        if (!client.sendFullResponse(500, "Internal Server Error", {{"Content-Type", "text/plain"}}, "Opening port failed!"))
            qWarning() << "sending response failed";
        return;
    }
}

void EspRemoteAgent::sendCloseResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query)
{
    if (!query.hasQueryItem("id"))
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "id missing"))
            qWarning() << "sending response failed";
        return;
    }
    const auto idStr = query.queryItemValue("id");
    bool ok{};
    const auto id = idStr.toInt(&ok);
    if (!ok)
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "could not parse id"))
            qWarning() << "sending response failed";
        return;
    }
    if (id < 0 || id >= m_ports.size())
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "id out of range"))
            qWarning() << "sending response failed";
        return;
    }

    (*std::next(std::begin(m_ports), id))->close();
    if (!client.sendFullResponse(200, "Ok", {{"Content-Type", "text/plain"}}, "Port closed successfully!"))
        qWarning() << "sending response failed";
}

void EspRemoteAgent::sendRebootResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query)
{
    if (!query.hasQueryItem("id"))
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "id missing"))
            qWarning() << "sending response failed";
        return;
    }
    const auto idStr = query.queryItemValue("id");
    bool ok{};
    const auto id = idStr.toInt(&ok);
    if (!ok)
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "could not parse id"))
            qWarning() << "sending response failed";
        return;
    }
    if (id < 0 || id >= m_ports.size())
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "id out of range"))
            qWarning() << "sending response failed";
        return;
    }

    if ((*std::next(std::begin(m_ports), id))->reboot())
    {
        if (!client.sendFullResponse(200, "Ok", {{"Content-Type", "text/plain"}}, "Reboot successfully!"))
            qWarning() << "sending response failed";
        return;
    }
    else
    {
        if (!client.sendFullResponse(500, "Internal Server Error", {{"Content-Type", "text/plain"}}, "Reboot failed!"))
            qWarning() << "sending response failed";
        return;
    }
}

void EspRemoteAgent::sendSetDTRResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query)
{
    if (!query.hasQueryItem("id"))
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "id missing"))
            qWarning() << "sending response failed";
        return;
    }
    const auto idStr = query.queryItemValue("id");
    bool ok{};
    const auto id = idStr.toInt(&ok);
    if (!ok)
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "could not parse id"))
            qWarning() << "sending response failed";
        return;
    }
    if (id < 0 || id >= m_ports.size())
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "id out of range"))
            qWarning() << "sending response failed";
        return;
    }

    if (!query.hasQueryItem("set"))
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "set missing"))
            qWarning() << "sending response failed";
        return;
    }
    bool set;
    if (const auto setStr = query.queryItemValue("set"); setStr == "true")
        set = true;
    else if (setStr == "false")
        set = false;
    else
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "could not parse set"))
            qWarning() << "sending response failed";
        return;
    }

    if ((*std::next(std::begin(m_ports), id))->setDataTerminalReady(set))
    {
        if (!client.sendFullResponse(200, "Ok", {{"Content-Type", "text/plain"}}, "Port set DTR successfully!"))
            qWarning() << "sending response failed";
        return;
    }
    else
    {
        if (!client.sendFullResponse(500, "Internal Server Error", {{"Content-Type", "text/plain"}}, "Set port DTR failed!"))
            qWarning() << "sending response failed";
        return;
    }
}

void EspRemoteAgent::sendSetRTSResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query)
{
    if (!query.hasQueryItem("id"))
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "id missing"))
            qWarning() << "sending response failed";
        return;
    }
    const auto idStr = query.queryItemValue("id");
    bool ok{};
    const auto id = idStr.toInt(&ok);
    if (!ok)
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "could not parse id"))
            qWarning() << "sending response failed";
        return;
    }
    if (id < 0 || id >= m_ports.size())
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "id out of range"))
            qWarning() << "sending response failed";
        return;
    }

    if (!query.hasQueryItem("set"))
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "set missing"))
            qWarning() << "sending response failed";
        return;
    }
    bool set;
    if (const auto setStr = query.queryItemValue("set"); setStr == "true")
        set = true;
    else if (setStr == "false")
        set = false;
    else
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "could not parse set"))
            qWarning() << "sending response failed";
        return;
    }

    if ((*std::next(std::begin(m_ports), id))->setRequestToSend(set))
    {
        if (!client.sendFullResponse(200, "Ok", {{"Content-Type", "text/plain"}}, "Port set RTS successfully!"))
            qWarning() << "sending response failed";
        return;
    }
    else
    {
        if (!client.sendFullResponse(500, "Internal Server Error", {{"Content-Type", "text/plain"}}, "Set port RTS failed!"))
            qWarning() << "sending response failed";
        return;
    }
}
