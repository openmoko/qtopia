qtopia_project(qtopia app)
TARGET=beaming
CONFIG+=qtopia_main no_quicklaunch

HEADERS		= ircontroller.h beaming.h
SOURCES		= ircontroller.cpp beaming.cpp main.cpp

depends(libraries/qtopiacomm)

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=beaming*
help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/beaming/*
pics.path=/pics/beaming
pics.hint=pics
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/Beaming.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=help pics desktop

pkg.desc=Beaming settings dialog for Qtopia.
pkg.domain=window,beaming
