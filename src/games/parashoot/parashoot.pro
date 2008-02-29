CONFIG		+= qtopiaapp

HEADERS		= interface.h man.h cannon.h base.h bullet.h helicopter.h
SOURCES		= interface.cpp man.cpp cannon.cpp base.cpp bullet.cpp helicopter.cpp main.cpp

TARGET		= parashoot

TRANSLATABLES = $${HEADERS} $${SOURCES}

help.files=$${QTOPIA_DEPOT_PATH}/help/html/parashoot*.html

help.path=/help/html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/parashoot.desktop
desktop.path=/apps/Games
pics.files=$${QTOPIA_DEPOT_PATH}/pics/parashoot/*
pics.path=/pics/parashoot
INSTALLS+=desktop help
PICS_INSTALLS+=pics
