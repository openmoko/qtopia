CONFIG += qtopiaapp

# Input
HEADERS += today.h todayoptions.h
SOURCES += today.cpp todayoptions.cpp main.cpp

INCLUDEPATH += 	library
DEPENDPATH += 	library

unix:LIBS += -lqtopia -Wl,-export-dynamic

INTERFACES = todayoptionsbase.ui

TRANSLATABLES = $$HEADERS \
                $$SOURCES \
                $$INTERFACES

TARGET = today

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=today.html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/today.desktop
desktop.path=/apps/Applications
pics.files=$${QTOPIA_DEPOT_PATH}/pics/today/*
pics.path=/pics/today
plugindir.files=$${QTOPIA_DEPOT_PATH}/plugins/today/.directory
plugindir.path=/plugins/today/
INSTALLS+=desktop plugindir
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Today provides a summary of important PIM data for your day.
