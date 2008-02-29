qtopia_project(qtopia app)
TARGET=startupflags
CONFIG+=qtopia_main no_quicklaunch

HEADERS		= startupflags.h
SOURCES		= startupflags.cpp main.cpp

TRANSLATABLES += startupflags.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/startupflags/StartupFlags.desktop
desktop.path=/apps/Devtools
desktop.hint=desktop

conf.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/startupflags/StartupFlags.conf
conf.path=/etc/default/Trolltech

script.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/startupflags/startupflags.sh
script.path=/bin
script.hint=script

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=*.html
help.hint=help

pics.files=pics/*
pics.path=/pics/startupflags
pics.hint=pics

INSTALLS+=desktop conf script help pics

pkg.desc=Startup flags editor for Qtopia.
pkg.domain=window
