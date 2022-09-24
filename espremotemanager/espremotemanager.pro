QT = core network websockets

TARGET = espremotemanager

TEMPLATE = app

CONFIG += console
CONFIG -= app_bundle

PROJECT_ROOT = ..

DESTDIR = $${OUT_PWD}/$${PROJECT_ROOT}/bin

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DBLIBS += webserver

HEADERS += \
    espremoteclient.h \
    espremotemanager.h

SOURCES += \
    espremoteclient.cpp \
    espremotemanager.cpp \
    main.cpp

OTHER_FILES += \
    espremotemanager.ini

include($${PROJECT_ROOT}/project.pri)
