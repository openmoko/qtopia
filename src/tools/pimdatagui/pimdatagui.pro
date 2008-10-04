!qbuild{
qtopia_project(qtopia app)

TARGET=pimdatagui
CONFIG += qtopia_main
}

# Input
SOURCES += main.cpp pimgen.cpp
HEADERS += pimgen.h
FORMS += pimgen.ui

pkg.desc=An application designed to aid testing of pim applications by generating large initial data sets.
pkg.domain=trusted

desktop.files=pimdatagui.desktop
desktop.path=/apps/Applications
desktop.hint=nct desktop
desktop.trtarget=pimdatagui-nct
INSTALLS+=desktop
