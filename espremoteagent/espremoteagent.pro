QT = core network serialport websockets

TARGET = espremoteagent

TEMPLATE = app

CONFIG += console
CONFIG -= app_bundle

PROJECT_ROOT = ..

DESTDIR = $${OUT_PWD}/$${PROJECT_ROOT}/bin

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DBLIBS += webserver

HEADERS += \
    espremoteagent.h \
    espremoteagentcontainers.h \
    espremoteport.h

SOURCES += \
    espremoteport.cpp \
    main.cpp \
    espremoteagent.cpp

OTHER_FILES += \
    espremoteagent.ini

include($${PROJECT_ROOT}/project.pri)
