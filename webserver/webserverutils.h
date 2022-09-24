#pragma once

#include <queue>

#include <QHostAddress>

#include "webserver_global.h"

QHostAddress WEBSERVER_EXPORT parseHostAddress(const QString &str);

template<typename T, typename Container=std::deque<T> >
class WEBSERVER_EXPORT iterable_queue : public std::queue<T,Container>
{
public:
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;

    iterator begin() { return this->c.begin(); }
    iterator end() { return this->c.end(); }
    const_iterator begin() const { return this->c.begin(); }
    const_iterator end() const { return this->c.end(); }
};
