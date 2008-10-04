!qbuild{
qtopia_project(qtopia app)
TARGET=gtalksettings
CONFIG+=qtopia_main no_quicklaunch
requires(enable_voip)
depends(libraries/qtopiaphone)
}

FORMS	= gtalksettingsbase.ui
HEADERS	= gtalksettings.h
SOURCES	= gtalksettings.cpp main.cpp


help.source=$$QTOPIA_DEPOT_PATH/help
help.files=gtalksettings*
help.hint=help
INSTALLS+=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/gtalksettings.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/gtalksettings/*
pics.path=/pics/gtalksettings
pics.hint=pics
INSTALLS+=pics

pkg.desc=Google Talk settings dialog for Qtopia.
pkg.domain=trusted
