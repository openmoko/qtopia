CONFIG		+= qtopiaapp

HEADERS		= wordgame.h
SOURCES		= wordgame.cpp main.cpp
INTERFACES	= newgamebase.ui rulesbase.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

TARGET		= wordgame


help.files=$${QTOPIA_DEPOT_PATH}/help/html/wordgame.html
help.path=/help/html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/wordgame.desktop
desktop.path=/apps/Games
pics.files=$${QTOPIA_DEPOT_PATH}/pics/wordgame/*
pics.path=/pics/wordgame
INSTALLS+=desktop help
PICS_INSTALLS+=pics
