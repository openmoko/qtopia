qtopia_project(qtopia plugin)
TARGET		= screensizeapplet

HEADERS		= screensize.h screensizeappletimpl.h
SOURCES		= screensize.cpp screensizeappletimpl.cpp

pics.files=$$QTOPIA_DEPOT_PATH/pics/screensize/*
pics.path=/pics/screensize
pics.hint=pics
INSTALLS+=pics

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/applets/screensizeapplet.desktop
desktop.path=/plugins/applets/
desktop.hint=desktop
INSTALLS+=desktop

