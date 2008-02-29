qtopia_project(qtopia plugin)
TARGET		= netmonapplet

FORMS	= netpw.ui
HEADERS		= netmon.h netmonappletimpl.h
SOURCES		= netmon.cpp netmonappletimpl.cpp

pics.files=$$QTOPIA_DEPOT_PATH/pics/netmon/*
pics.path=/pics/netmon
pics.hint=pics
INSTALLS+=pics

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/applets/netmonapplet.desktop
desktop.path=/plugins/applets/
desktop.hint=desktop
INSTALLS+=desktop

