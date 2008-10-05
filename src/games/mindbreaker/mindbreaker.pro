!qbuild {
qtopia_project(qtopia app)
TARGET=mindbreaker
CONFIG+=qtopia_main
}

HEADERS		= mindbreaker.h
SOURCES		= mindbreaker.cpp main.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=mindbreaker.html
help.hint=help
INSTALLS+=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Games/mindbreaker.desktop
desktop.path=/apps/Games
desktop.hint=desktop
INSTALLS+=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/mindbreaker/*
pics.path=/pics/mindbreaker
pics.hint=pics
INSTALLS+=pics

pkg.description=Try to guess the positions and colors of the pegs.
pkg.domain=trusted

