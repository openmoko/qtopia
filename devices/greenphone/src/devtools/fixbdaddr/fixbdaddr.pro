qtopia_project(qtopia app)
TARGET=fixbdaddr
CONFIG+=qtopia_main no_quicklaunch

HEADERS		= fixbdaddr.h
SOURCES		= fixbdaddr.cpp main.cpp

TRANSLATABLES += fixbdaddr.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/fixbdaddr/fixbdaddr.desktop
desktop.path=/apps/Devtools
desktop.hint=desktop
INSTALLS+=desktop

pkg.desc=Greenphone unique bdaddr fixer
pkg.domain=window

depends(libraries/qtopiaphone)
