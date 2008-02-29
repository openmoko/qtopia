qtopia_project(qtopia plugin)
TARGET		= mountmonapplet

HEADERS	=	mountmon.h mountmonimpl.h
SOURCES	=	mountmon.cpp mountmonimpl.cpp

pics.files=$$QTOPIA_DEPOT_PATH/pics/mountmon/*
pics.path=/pics/mountmon
pics.hint=pics
INSTALLS+=pics

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/applets/mountmonapplet.desktop
desktop.path=/plugins/applets/
desktop.hint=desktop
INSTALLS+=desktop

