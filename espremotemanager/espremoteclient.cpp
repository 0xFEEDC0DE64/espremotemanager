#include "espremoteclient.h"

#include <QWebSocket>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

EspRemoteClient::EspRemoteClient(QWebSocket *websocket, EspRemoteManager &manager, QObject *parent) :
    QObject{parent},
    m_websocket{websocket},
    m_manager{manager}
{
    qDebug() << "connected" << m_websocket->peerAddress().toString() << m_websocket->peerPort();

    connect(m_websocket.get(), &QWebSocket::disconnected, this, &QObject::deleteLater);
    connect(m_websocket.get(), &QWebSocket::textMessageReceived, this, &EspRemoteClient::textMessageReceived);
}

EspRemoteClient::~EspRemoteClient()
{
    qDebug() << "disconnected" << m_websocket->peerAddress().toString() << m_websocket->peerPort();
}

QString EspRemoteClient::peer() const
{
    return QStringLiteral("%0:%1").arg(m_websocket->peerAddress().toString()).arg(m_websocket->peerPort());
}

QString EspRemoteClient::path() const
{
    return m_websocket->requestUrl().path();
}

QString EspRemoteClient::logOutput() const
{
    QString str;
    for (const auto &line : m_logOutput)
    {
        if (!str.isEmpty())
            str += "\n";
        str += line.toHtmlEscaped();
    }
    return str;
}

void EspRemoteClient::reboot()
{
    m_websocket->sendTextMessage(QJsonDocument{QJsonObject{
        {"type", "reboot"}
    }}.toJson());
}

void EspRemoteClient::textMessageReceived(const QString &message)
{
//    qDebug() << message;

    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(message.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError)
    {
        qWarning() << "could not parse json command:" << error.errorString();
        return;
    }

    if (!doc.isObject())
    {
        qWarning() << "json command is not an object";
        return;
    }

    const auto obj = doc.object();

    if (!obj.contains("type"))
    {
        qWarning() << "json command does not contain a type";
        return;
    }

    const auto typeVal = obj.value("type");

    if (!typeVal.isString())
    {
        qWarning() << "json command type is not a string";
        return;
    }

    const auto type = typeVal.toString();

    if (type == "log")
    {
        if (!obj.contains("line"))
        {
            qWarning() << "json command does not contain a line";
            return;
        }

        const auto lineVal = obj.value("line");

        if (!lineVal.isString())
        {
            qWarning() << "json command line is not a string";
            return;
        }

        auto line = lineVal.toString();

        logReceived(std::move(line));
    }
    else
        qWarning() << "unknown command type" << type;
}

void EspRemoteClient::logReceived(QString &&line)
{
    m_logOutput.push(std::move(line));

    while (m_logOutput.size() > 10)
        m_logOutput.pop();
}
