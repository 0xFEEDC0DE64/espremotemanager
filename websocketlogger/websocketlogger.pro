QT = core network websockets

TARGET = websocketlogger

TEMPLATE = app

CONFIG += console
CONFIG -= app_bundle

PROJECT_ROOT = ..

DESTDIR = $${OUT_PWD}/$${PROJECT_ROOT}/bin

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DBLIBS +=

HEADERS +=

SOURCES += \
    main.cpp

OTHER_FILES +=

include($${PROJECT_ROOT}/project.pri)
