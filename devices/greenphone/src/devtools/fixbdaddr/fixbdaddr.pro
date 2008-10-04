!qbuild{
qtopia_project(qtopia app)
TARGET=fixbdaddr
CONFIG+=qtopia_main
depends(libraries/qtopiaphone)
# can't install the .desktop file before the .directory file has been processed
depends(devtools,fake)
}

HEADERS		= fixbdaddr.h
SOURCES		= fixbdaddr.cpp main.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/fixbdaddr/fixbdaddr.desktop
desktop.path=/apps/Devtools
desktop.hint=desktop
INSTALLS+=desktop

pkg.desc=Greenphone unique bdaddr fixer
pkg.domain=trusted

