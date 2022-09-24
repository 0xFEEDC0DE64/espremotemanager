#include <QCoreApplication>
#include <QSettings>
#include <QWebSocketServer>

#include "webserverutils.h"
#include "espremotemanager.h"

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

    QSettings settings{"espremotemanager.ini", QSettings::IniFormat};

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

    EspRemoteManager manager{websocketServer};

    if (!manager.listen(webserverListen, webserverPort))
        qFatal("could not start webserver listening %s", qPrintable(manager.errorString()));

    if (!websocketServer.listen(websocketListen, websocketPort))
        qFatal("could not start webserver listening %s", qPrintable(manager.errorString()));

    return app.exec();
}
