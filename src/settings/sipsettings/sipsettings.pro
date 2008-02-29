qtopia_project(qtopia app)
TARGET=sipsettings
CONFIG+=qtopia_main no_quicklaunch
# This depends on dissipate2 which is disabled in singleexec builds but
# singleexec_reader.pri can't figure out to disable it so we have to
# manually disable it.
CONFIG+=no_singleexec

INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/3rdparty/libraries

FORMS	= sipsettingsbase.ui 
HEADERS	= sipsettings.h
SOURCES	= sipsettings.cpp main.cpp

depends(3rdparty/libraries/dissipate2)
depends(libraries/qtopiaphone)

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=sipsettings*
help.hint=help
INSTALLS+=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/sipsettings.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/sipsettings/*
pics.path=/pics/sipsettings
pics.hint=pics
INSTALLS+=pics

pkg.desc=SIP settings dialog for Qtopia.
pkg.domain=window,phonecomm
