qtopia_project(qtopia plugin)
TARGET		= volumeapplet

HEADERS		= volume.h volumeappletimpl.h
SOURCES		= volume.cpp volumeappletimpl.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/applets/volumeapplet.desktop
desktop.path=/plugins/applets/
desktop.hint=desktop
INSTALLS+=desktop

