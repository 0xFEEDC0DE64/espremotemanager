#include "espremoteport.h"

#include <QSerialPort>
#include <QWebSocket>
#include <QTimerEvent>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

EspRemotePort::EspRemotePort(SerialPortConfig &&config, QObject *parent) :
    QObject{parent},
    m_config{std::move(config)},
    m_port{std::make_unique<QSerialPort>(m_config.port)},
    m_websocket{m_config.url.isEmpty() ? nullptr : std::make_unique<QWebSocket>(QString{}, QWebSocketProtocol::VersionLatest, this)}
{
    connect(m_port.get(), &QSerialPort::readyRead, this, &EspRemotePort::serialReadyRead);

    if (m_websocket)
    {
        connect(m_websocket.get(), &QWebSocket::connected, this, &EspRemotePort::websocketConnected);
        connect(m_websocket.get(), &QWebSocket::disconnected, this, &EspRemotePort::websocketDisconnected);
        connect(m_websocket.get(), qOverload<QAbstractSocket::SocketError>(&QWebSocket::error), this, &EspRemotePort::websocketError);
        connect(m_websocket.get(), &QWebSocket::textMessageReceived, this, &EspRemotePort::websocketTextMessageReceived);
        qDebug() << "connecting to" << m_config.url;
        m_websocket->open(m_config.url);
    }

    tryOpen();
}

EspRemotePort::~EspRemotePort() = default;

QString EspRemotePort::status() const
{
    if (m_port->isOpen())
        return tr("Open");
    else
        return tr("Not open");
}

QString EspRemotePort::logOutput() const
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

bool EspRemotePort::reboot()
{
    bool set = m_port->isDataTerminalReady();

    Q_ASSERT(m_port);

    if (!m_port->setDataTerminalReady(!set))
        return false;

    QTimer::singleShot(100, m_port.get(), [set,port=m_port.get()](){
        if (!port->setDataTerminalReady(set))
            qWarning() << "reboot failed";
    });

    return true;
}

bool EspRemotePort::setDataTerminalReady(bool set)
{
    return m_port->setDataTerminalReady(set);
}

bool EspRemotePort::isDataTerminalReady()
{
    return m_port->isDataTerminalReady();
}

bool EspRemotePort::setRequestToSend(bool set)
{
    return m_port->setRequestToSend(set);
}

bool EspRemotePort::isRequestToSend()
{
    return m_port->isRequestToSend();
}

bool EspRemotePort::tryOpen()
{
    m_port->close();

    if (!m_port->setBaudRate(m_config.baudrate))
        qWarning() << "could not set baud rate" << m_config.baudrate;

    if (!m_port->open(QIODevice::ReadWrite))
    {
        m_message = tr("Could not open port because %0").arg(m_port->errorString());
        qWarning() << m_message;
        return false;
    }

    return true;
}

void EspRemotePort::close()
{
    m_port->close();
}

void EspRemotePort::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_reconnectTimerId)
    {
        m_reconnectTimerId = -1;

        if (!m_config.url.isEmpty())
        {
            Q_ASSERT(m_websocket);
            qDebug() << "reconnecting to" << m_config.url;
            m_websocket->open(m_config.url);
        }
    }
    else
        QObject::timerEvent(event);
}

void EspRemotePort::serialReadyRead()
{
    while (m_port->canReadLine())
    {
        auto line = m_port->readLine();

        if (line.endsWith('\n'))
        {
            line.chop(1);
            if (line.endsWith('\r'))
                line.chop(1);
        }

//        qDebug() << line;

        if (m_websocket)
        {
            m_websocket->sendTextMessage(QJsonDocument{QJsonObject{
                {"type", "log"},
                {"line", QString{line}},
            }}.toJson());
        }

        m_logOutput.push(std::move(line));

        while (m_logOutput.size() > 10)
            m_logOutput.pop();
    }
}

void EspRemotePort::websocketConnected()
{
    qDebug() << "called";

    if (m_reconnectTimerId != -1)
        killTimer(m_reconnectTimerId);
}

void EspRemotePort::websocketDisconnected()
{
    qDebug() << "called";
}

void EspRemotePort::websocketError(QAbstractSocket::SocketError error)
{
    qDebug() << "called" << error;

    if (m_reconnectTimerId != -1)
        killTimer(m_reconnectTimerId);

    m_reconnectTimerId = startTimer(5000);
}

void EspRemotePort::websocketTextMessageReceived(const QString &message)
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

    if (type == "reboot")
    {
        reboot();
    }
    else
        qWarning() << "unknown command type" << type;
}
