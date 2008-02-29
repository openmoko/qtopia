qtopia_project(qtopia plugin)
TARGET		= brightnessapplet
CONFIG += no_tr

HEADERS		= brightness.h brightnessappletimpl.h
SOURCES		= brightness.cpp brightnessappletimpl.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/plugins/applets/brightnessapplet.desktop
desktop.path=/plugins/applets/
desktop.hint=desktop
INSTALLS+=desktop

depends(settings/light-and-power)

