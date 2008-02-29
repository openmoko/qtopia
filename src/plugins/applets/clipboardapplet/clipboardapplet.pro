qtopia_project(qtopia plugin)
TARGET		= clipboardapplet

HEADERS		= clipboard.h clipboardappletimpl.h
SOURCES		= clipboard.cpp clipboardappletimpl.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/applets/clipboardapplet.desktop
desktop.path=/plugins/applets/
desktop.hint=desktop
INSTALLS+=desktop

