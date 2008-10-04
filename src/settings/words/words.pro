!qbuild {
qtopia_project(qtopia app)
TARGET=words
CONFIG+=qtopia_main
depends(3rdparty/libraries/inputmatch)
}

HEADERS		= words.h
SOURCES		= words.cpp main.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=words*
help.hint=help
INSTALLS+=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/words/*
pics.path=/pics/words
pics.hint=pics
INSTALLS+=pics
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/words.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop

pkg.desc=Words settings dialog for Qtopia.
pkg.domain=trusted
