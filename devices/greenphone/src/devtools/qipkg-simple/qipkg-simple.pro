qtopia_project(qtopia app)
TARGET=qipkg
CONFIG+=qtopia_main
CONFIG+=no_singleexec
CONFIG+=no_quicklaunch
CONFIG+=no_tr

FORMS=qipkgbase.ui
HEADERS=qipkg-simple.h
SOURCES=main.cpp qipkg-simple.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/qipkg-simple/qipkg-simple.desktop
desktop.path=/apps/Settings
desktop.hint=desktop

pics.files=pics/*
pics.path=/pics/qipkg
pics.hint=pics

script.files=$$QTOPIA_DEPOT_PATH/devices/greenphone/src/devtools/qipkg-simple/pkg.sh
script.path=/bin
script.hint=script

INSTALLS+=desktop pics script

pkg.name=qipkg
pkg.desc=Simple ipkg Installer
pkg.version=1.0.0-1
pkg.maintainer=Trolltech (www.trolltech.com)
pkg.license=GPL
pkg.domain=window
