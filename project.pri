CONFIG += c++17

DEFINES += QT_DEPRECATED_WARNINGS \
           QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
           QT_MESSAGELOGCONTEXT

equals(TEMPLATE, "lib") {
    win32: DESTDIR = $${OUT_PWD}/$${PROJECT_ROOT}/bin
    else: DESTDIR = $${OUT_PWD}/$${PROJECT_ROOT}/lib
}

!isEmpty(DBLIBS) {
    win32: LIBS += -L$${OUT_PWD}/$${PROJECT_ROOT}/bin
    else: LIBS += -Wl,-rpath=\\\$$ORIGIN/../lib -L$${OUT_PWD}/$${PROJECT_ROOT}/lib
}

contains(DBLIBS, webserver) {
    LIBS += -lwebserver

    INCLUDEPATH += $$PWD/webserver
    DEPENDPATH += $$PWD/webserver
}

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
