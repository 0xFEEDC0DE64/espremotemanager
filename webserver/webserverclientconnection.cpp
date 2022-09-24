#include "webserverclientconnection.h"

#include <utility>

#include <QTcpSocket>

#include "abstractwebserver.h"

WebserverClientConnection::WebserverClientConnection(QTcpSocket &socket, AbstractWebserver &webserver, QObject *parent) :
    QObject{parent},
    m_socket{&socket},
    m_webserver{webserver}
{
//    qDebug() << "connected";
    m_socket->setParent(this);

    connect(m_socket.get(), &QTcpSocket::readyRead, this, &WebserverClientConnection::readyRead);
    connect(m_socket.get(), &QTcpSocket::disconnected, this, &QObject::deleteLater);
}

WebserverClientConnection::~WebserverClientConnection()
{
//    qDebug() << "disconnected";
}

bool WebserverClientConnection::sendResponseHeaders(int status, const QByteArray &message, const QMap<QByteArray, QByteArray> &responseHeaders)
{
    if (m_status != Response)
        return false;

    if (!writeLine(m_request.protocol + ' ' + QString::number(status).toUtf8() + ' ' + message))
        return false;

    for (auto iter = std::begin(responseHeaders); iter != std::end(responseHeaders); iter++)
        if (!writeLine(iter.key() + ": " + iter.value()))
            return false;

    if (!writeLine({}))
        return false;

    return true;
}

bool WebserverClientConnection::sendFullResponse(int status, const QByteArray &message,
                                                 QMap<QByteArray, QByteArray> responseHeaders, const QByteArray &response)
{
    if (m_status != Response)
    {
        qWarning() << "status not response";
        return false;
    }

    const auto containsKey = [&](auto key){
        for (auto iter = std::begin(responseHeaders); iter != std::end(responseHeaders); iter++)
            if (iter.key().compare(key, Qt::CaseInsensitive) == 0)
                return true;
        return false;
    };

    if (!containsKey("Connection"))
        responseHeaders.insert("Connection", "keep");

    if (!response.isEmpty() && !containsKey("Content-Length"))
        responseHeaders.insert("Content-Length", QString::number(response.size()).toUtf8());

    if (!sendResponseHeaders(status, message, responseHeaders))
        return false;

    m_socket->write(response);
    m_socket->flush();

    m_request.clear();
    m_status = RequestLine;

    return true;
}

void WebserverClientConnection::readyRead()
{
    while (m_socket->canReadLine())
    {
        auto line = m_socket->readLine();
        if (line.endsWith('\n'))
        {
            line.chop(1);
            if (line.endsWith('\r'))
                line.chop(1);
        }

//        qDebug() << line;

        switch (m_status)
        {
        case RequestLine:
        {
            auto parts = line.split(' ');
            if (parts.size() < 3)
            {
                qWarning() << "invalid request line" << line;
                m_socket->close();
                return;
            }

            m_request.method = parts.takeFirst();
            m_request.path = parts.takeFirst();
            m_request.protocol = parts.join(' ');

            m_status = RequestHeaders;
            continue;
        }
        case RequestHeaders:
        {
            if (line.isEmpty())
            {
                m_status = Response;

                m_webserver.requestReceived(*this, m_request);
            }
            else
            {
                const auto index = line.indexOf(": ");
                if (index == -1)
                    qWarning() << "could not parse request header" << line;
                else
                    m_request.headers.insert(line.left(index), line.mid(index + 2));
            }

            continue;
        default:
            qWarning() << "received data in unexpected state" << m_status;
        }
        }
    }
}

bool WebserverClientConnection::writeLine(const QByteArray &buf)
{
//    qDebug() << buf;
    m_socket->write(buf);
    m_socket->write("\r\n");

    return true;
}
