#include <QCoreApplication>
#include <QWebSocket>
#include <QCommandLineParser>
#include <QTimer>

#include <cstdio>

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};
    QCoreApplication::setApplicationName("websocketlogger");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("V3 Produktionstool");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("url", QCoreApplication::translate("main", "Websocket url"));

    parser.process(app);

    const QStringList args = parser.positionalArguments();

    if (args.isEmpty())
        qFatal("no url specified!");

    QUrl url = QUrl::fromUserInput(args.first());
    if (!url.isValid())
        qFatal("invalid url");

    QWebSocket socket;

    const auto connect = [&](){
        qDebug() << "connecting to" << url;
        socket.open(url);
    };

    QObject::connect(&socket, &QWebSocket::connected, [](){
        qDebug() << "connected";
    });
    QObject::connect(&socket, &QWebSocket::disconnected, [&](){
        qDebug() << "disconnected";
        QTimer::singleShot(5000, &socket, connect);
    });
    QObject::connect(&socket, &QWebSocket::textMessageReceived, [](const QString &message){
        printf("%s\n", message.toStdString().c_str());
    });
    QObject::connect(&socket, &QWebSocket::binaryMessageReceived, [](){
        qDebug() << "binaryMessageReceived";
    });

    connect();

    return app.exec();
}
