#include <QCoreApplication>
#include <QSettings>
#include <QWebSocketServer>
#include <QUrl>

#include "espremoteagent.h"
#include "espremoteagentcontainers.h"
#include "webserverutils.h"

int main(int argc, char *argv[])
{
    qSetMessagePattern(QStringLiteral("%{time dd.MM.yyyy HH:mm:ss.zzz} "
                                      "["
                                      "%{if-debug}D%{endif}"
                                      "%{if-info}I%{endif}"
                                      "%{if-warning}W%{endif}"
                                      "%{if-critical}C%{endif}"
                                      "%{if-fatal}F%{endif}"
                                      "] "
                                      "%{function}(): "
                                      "%{message}"));

    QCoreApplication app{argc, argv};

    QSettings settings{"espremoteagent.ini", QSettings::IniFormat};

    std::vector<SerialPortConfig> serialPortConfigs;

    const auto probePort = [&](auto group){
        auto port = settings.value(QStringLiteral("%0/port").arg(group)).toString();
        if (port.isEmpty())
            return;

        QUrl url;
        if (auto urlStr = settings.value(QStringLiteral("%0/url").arg(group)).toString(); !urlStr.isEmpty())
            url = QUrl{std::move(urlStr)};

        int baudrate{};
        bool ok{};
        baudrate = settings.value(QStringLiteral("%0/baudrate").arg(group)).toInt(&ok);
        if (!ok)
            qFatal("could not parse baudrate for %s", qPrintable(group));

        serialPortConfigs.emplace_back(SerialPortConfig{
            .port=std::move(port),
            .baudrate=baudrate,
            .url=std::move(url)
        });
    };

    probePort("PortA");
    probePort("PortB");

    QHostAddress webserverListen = parseHostAddress(settings.value("Webserver/listen").toString());
    int webserverPort;
    {
        bool ok{};
        webserverPort = settings.value("Webserver/port", 1234).toInt(&ok);
        if (!ok)
            qFatal("could not parse webserver port");
    }

    QHostAddress websocketListen = parseHostAddress(settings.value("Websocket/listen").toString());
    int websocketPort;
    {
        bool ok{};
        websocketPort = settings.value("Websocket/port", 1234).toInt(&ok);
        if (!ok)
            qFatal("could not parse webserver port");
    }

    QWebSocketServer websocketServer{"dafuq", QWebSocketServer::NonSecureMode};

    EspRemoteAgent agent{websocketServer, std::move(serialPortConfigs)};

    if (!agent.listen(webserverListen, webserverPort))
        qFatal("could not start listening %s", qPrintable(agent.errorString()));

    if (!websocketServer.listen(websocketListen, websocketPort))
        qFatal("could not start webserver listening %s", qPrintable(websocketServer.errorString()));

    return app.exec();
}
