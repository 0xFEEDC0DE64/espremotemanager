#include "espremotemanager.h"

#include <QDebug>
#include <QUrl>
#include <QUrlQuery>
#include <QWebSocketServer>

#include "webservercontainer.h"
#include "webserverclientconnection.h"
#include "espremoteclient.h"

EspRemoteManager::EspRemoteManager(QWebSocketServer &websocketServer, QObject *parent) :
    AbstractWebserver{parent},
    m_websocketServer{websocketServer}
{
    connect(&m_websocketServer, &QWebSocketServer::newConnection, this, &EspRemoteManager::newConnection);
}

EspRemoteManager::~EspRemoteManager() = default;

void EspRemoteManager::requestReceived(WebserverClientConnection &client, const Request &request)
{
    QUrl url{request.path};
    QUrlQuery query{url};

    if (url.path() == "/")
    {
        sendRootResponse(client, url, query);
    }
    else if (url.path() == "/rebootAll")
    {
        sendRebootAllResponse(client, url, query);
    }
    else if (url.path() == "/reboot")
    {
        sendRebootResponse(client, url, query);
    }
    else
        if (!client.sendFullResponse(404, "Not Found", {{"Content-Type", "text/plain"}}, "The requested path \"" + request.path + "\" was not found."))
            qWarning() << "sending response failed";
}

void EspRemoteManager::newConnection()
{
    while (const auto socket = m_websocketServer.nextPendingConnection())
    {
        auto ö = new EspRemoteClient(socket, *this, this);
        connect(ö, &QObject::destroyed, this, &EspRemoteManager::clientDestroyed);
        m_clients.emplace_back(ö);
    }
}

void EspRemoteManager::clientDestroyed(QObject *object)
{
    m_clients.erase(std::remove(std::begin(m_clients), std::end(m_clients), object), std::end(m_clients));
}

void EspRemoteManager::sendRootResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query)
{
    QString content =
            "<!doctype html>"
            "<html lang=\"en\">"
                "<head>"
                    "<meta charset=\"utf-8\" />"
                    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />"
                    "<title>ESP Remote Manager</title>"
                    "<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.2.1/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-iYQeCzEYFbKjA/T2uDLTpkwGzCiq6soy8tYaI1GyVh/UjpbCx/TYkiZhlZB6+fzT\" crossorigin=\"anonymous\" />"
                    "<link rel=\"stylesheet\" href=\"https://cdn.datatables.net/1.12.1/css/dataTables.bootstrap5.min.css\" integrity=\"sha384-V05SibXwq2x9UKqEnsL0EnGlGPdbHwwdJdMjmp/lw3ruUri9L34ioOghMTZ8IHiI\" crossorigin=\"anonymous\">"
                "</head>"
                "<body>"
                    "<h1>ESP Remote Manager</h1>"
                    "<a href=\"rebootAll\">Reboot all</a>"
                    "<table class=\"table table-striped table-bordered table-sm\" style=\"width: initial;\">"
                        "<thead>"
                            "<tr>"
                                "<th>Peer</th>"
                                "<th>WS Path</th>"
                                "<th>Actions</th>"
                                "<th>Log</th>"
                            "</tr>"
                        "</thead>"
                        "<tbody>";

    for (auto port : m_clients)
    {
        content += QStringLiteral("<tr>"
                                      "<td>%0</td>"
                                      "<td>%1</td>"
                                      "<td><a href=\"reboot?peer=%0\">Reboot</a></td>"
                                      "<td><pre>%2</pre></td>"
                                  "</tr>")
                .arg(port->peer())
                .arg(port->path())
                .arg(port->logOutput());
    }

    content +=          "</tbody>"
                    "</table>"
                    "<script src=\"https://code.jquery.com/jquery-3.6.1.min.js\" integrity=\"sha384-i61gTtaoovXtAbKjo903+O55Jkn2+RtzHtvNez+yI49HAASvznhe9sZyjaSHTau9\" crossorigin=\"anonymous\"></script>"
                    "<script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.2.1/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-u1OknCvxWvY5kfmNBILK2hRnQC3Pr17a+RTT6rIHI7NnikvbZlHgTPOOmMi466C8\" crossorigin=\"anonymous\"></script>"
                    "<script src=\"https://cdn.datatables.net/1.12.1/js/jquery.dataTables.min.js\" integrity=\"sha384-ZuLbSl+Zt/ry1/xGxjZPkp9P5MEDotJcsuoHT0cM8oWr+e1Ide//SZLebdVrzb2X\" crossorigin=\"anonymous\"></script>"
                    "<script src=\"https://cdn.datatables.net/1.12.1/js/dataTables.bootstrap5.min.js\" integrity=\"sha384-jIAE3P7Re8BgMkT0XOtfQ6lzZgbDw/02WeRMJvXK3WMHBNynEx5xofqia1OHuGh0\" crossorigin=\"anonymous\"></script>"
                    "<script>"
                        "$(document).ready(function () {"
                            "$('table').DataTable({"
                                "filter: false,"
                                "filtering: false,"
                                "paging: false,"
                                "info: false,"
                            "});"
                        "});"
                    "</script>"
                "</body>"
            "</html>";

    if (!client.sendFullResponse(200, "Ok", {{"Content-Type", "text/html"}}, content.toUtf8()))
        qWarning() << "sending response failed";
}

void EspRemoteManager::sendRebootAllResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query)
{
    for (auto client : m_clients)
        client->reboot();

    if (!client.sendFullResponse(200, "Ok", {{"Content-Type", "text/plain"}}, "peer reboot commands sent!"))
        qWarning() << "sending response failed";
}

void EspRemoteManager::sendRebootResponse(WebserverClientConnection &client, const QUrl &url, const QUrlQuery &query)
{
    if (!query.hasQueryItem("peer"))
    {
        if (!client.sendFullResponse(400, "Bad Request", {{"Content-Type", "text/plain"}}, "peer missing"))
            qWarning() << "sending response failed";
        return;
    }
    const auto peer = query.queryItemValue("peer");
    auto iter = std::find_if(std::begin(m_clients), std::end(m_clients), [&peer](EspRemoteClient *client){ return client->peer() == peer; });
    if (iter == std::end(m_clients))
    {
        if (!client.sendFullResponse(404, "Bad Request", {{"Content-Type", "text/plain"}}, "peer not found"))
            qWarning() << "sending response failed";
        return;
    }

    (*iter)->reboot();

    if (!client.sendFullResponse(200, "Ok", {{"Content-Type", "text/plain"}}, "peer reboot command sent!"))
        qWarning() << "sending response failed";
}
