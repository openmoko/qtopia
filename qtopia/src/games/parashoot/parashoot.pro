CONFIG		+= qtopiaapp

HEADERS		= interface.h man.h cannon.h base.h bullet.h helicopter.h
SOURCES		= interface.cpp man.cpp cannon.cpp base.cpp bullet.cpp helicopter.cpp main.cpp

TARGET		= parashoot

TRANSLATABLES = $${HEADERS} $${SOURCES}

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=parashoot*.html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/parashoot.desktop
desktop.path=/apps/Games
pics.files=$${QTOPIA_DEPOT_PATH}/pics/parashoot/*
pics.path=/pics/parashoot
INSTALLS+=desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics
