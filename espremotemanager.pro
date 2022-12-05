TEMPLATE = subdirs

SUBDIRS += \
    espremoteagent \
    espremotemanager \
    webserver \
    websocketlogger

espremoteagent.depends += webserver
espremotemanager.depends += webserver
