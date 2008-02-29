qtopia_project(qtopia app)
TARGET=worldtime
CONFIG+=qtopia_main no_quicklaunch

#UI_HEADERS_DIR	= $${UI_HEADERS_DIR}/qtopia

HEADERS		= worldtime.h cityinfo.h
SOURCES		= worldtime.cpp main.cpp cityinfo.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=worldtime*
help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/worldtime/*
pics.path=/pics/worldtime
pics.hint=pics
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/worldtime.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=help pics desktop

pkg.desc=The time-zone manager for Qtopia.
pkg.domain=window,datetime
