TEMPLATE = subdirs

SUBDIRS += \
    espremoteagent \
    espremotemanager \
    webserver

espremoteagent.depends += webserver
espremotemanager.depends += webserver
