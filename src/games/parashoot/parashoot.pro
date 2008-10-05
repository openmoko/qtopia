!qbuild {
qtopia_project(qtopia app)
TARGET=parashoot
CONFIG+=qtopia_main
}

HEADERS		= animateditem.h interface.h man.h cannon.h base.h bullet.h helicopter.h
SOURCES		= animateditem.cpp interface.cpp man.cpp cannon.cpp base.cpp bullet.cpp helicopter.cpp main.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=parashoot*.html
help.hint=help
INSTALLS+=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Games/parashoot.desktop
desktop.path=/apps/Games
desktop.hint=desktop
INSTALLS+=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/parashoot/*
pics.path=/pics/parashoot
pics.hint=pics
INSTALLS+=pics
sounds.files=$$QTOPIA_DEPOT_PATH/sounds/parashoot/*
sounds.path=/sounds/parashoot
INSTALLS+=sounds

pkg.desc=Shoot the parachutists before they land.
pkg.domain=trusted
