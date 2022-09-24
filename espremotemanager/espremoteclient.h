#pragma once

#include <memory>

#include <QObject>

#include "webserverutils.h"

class QWebSocket;
class EspRemoteManager;

class EspRemoteClient : public QObject
{
    Q_OBJECT

public:
    explicit EspRemoteClient(QWebSocket *websocket, EspRemoteManager &manager, QObject *parent = nullptr);
    ~EspRemoteClient() override;

    QString peer() const;
    QString path() const;
    QString logOutput() const;

    void reboot();

private slots:
    void textMessageReceived(const QString &message);

private:
    void logReceived(QString &&line);

    const std::unique_ptr<QWebSocket> m_websocket;
    EspRemoteManager &m_manager;

    iterable_queue<QString> m_logOutput;
};
