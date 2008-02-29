qtopia_project(qtopia plugin)
TARGET		= clockapplet

HEADERS		= clock.h clockappletimpl.h
SOURCES		= clock.cpp clockappletimpl.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/applets/clockapplet.desktop
desktop.path=/plugins/applets/
desktop.hint=desktop
INSTALLS+=desktop

