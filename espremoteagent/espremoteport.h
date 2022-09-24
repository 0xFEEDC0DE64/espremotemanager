#pragma once

#include <memory>

#include <QObject>
#include <QAbstractSocket>

#include "espremoteagentcontainers.h"
#include "webserverutils.h"

class QSerialPort;
class QWebSocket;

class EspRemotePort : public QObject
{
    Q_OBJECT

public:
    explicit EspRemotePort(SerialPortConfig &&config, QObject *parent = nullptr);
    ~EspRemotePort() override;

    QString port() const { return m_config.port; }
    QString status() const;
    QString message() const { return m_message; }
    QString logOutput() const;

    bool reboot();

    bool setDataTerminalReady(bool set);
    bool isDataTerminalReady();

    bool setRequestToSend(bool set);
    bool isRequestToSend();

    bool tryOpen();
    void close();

protected:
    void timerEvent(QTimerEvent *event) override;

private slots:
    void serialReadyRead();
    void websocketConnected();
    void websocketDisconnected();
    void websocketError(QAbstractSocket::SocketError error);
    void websocketTextMessageReceived(const QString &message);

private:
    SerialPortConfig m_config;
    const std::unique_ptr<QSerialPort> m_port;
    const std::unique_ptr<QWebSocket> m_websocket;
    QString m_message;
    iterable_queue<QString> m_logOutput;

    int m_reconnectTimerId{-1};
};
