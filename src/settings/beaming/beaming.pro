!qbuild {
qtopia_project(qtopia app)
TARGET=beaming
CONFIG+=qtopia_main
depends(libraries/qtopiacomm/ir)
}

HEADERS		= ircontroller.h beaming.h
SOURCES		= ircontroller.cpp beaming.cpp main.cpp


help.source=$$QTOPIA_DEPOT_PATH/help
help.files=beaming*
help.hint=help
INSTALLS+=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/beaming/*
pics.path=/pics/beaming
pics.hint=pics
INSTALLS+=pics
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/beaming.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop

beam.files=$$QTOPIA_DEPOT_PATH/etc/beam
beam.path=/etc
INSTALLS+=beam

pkg.desc=Beaming settings dialog for Qtopia.
pkg.domain=trusted
