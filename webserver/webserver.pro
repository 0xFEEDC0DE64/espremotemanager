QT += core network
QT -= gui widgets

TARGET = webserver
TEMPLATE = lib

PROJECT_ROOT = ..

DEFINES += WEBSERVER_LIBRARY

HEADERS += \
    abstractwebserver.h \
    webservercontainer.h \
    webserverclientconnection.h \
    webserver_global.h \
    webserverutils.h

SOURCES += \
    abstractwebserver.cpp \
    webserverclientconnection.cpp \
    webserverutils.cpp

include($${PROJECT_ROOT}/project.pri)
