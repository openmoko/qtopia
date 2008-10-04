!qbuild{
qtopia_project(qtopia app)
TARGET=rotation
CONFIG+=qtopia_main
}

FORMS	= rotationsettingsbase.ui
HEADERS		= rotation.h
SOURCES		= rotation.cpp main.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=rotation.html
help.hint=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/rotation.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/rotation/*
pics.path=/pics/rotation
pics.hint=pics
INSTALLS+=help desktop pics

pkg.description=Screen rotation application
pkg.domain=trusted
