qtopia_project(qtopia plugin)
TARGET=batteryapplet

HEADERS		= battery.h batterystatus.h batteryappletimpl.h
SOURCES		= battery.cpp batterystatus.cpp batteryappletimpl.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/applets/batteryapplet.desktop
desktop.path=/plugins/applets/
desktop.hint=desktop
INSTALLS+=desktop

