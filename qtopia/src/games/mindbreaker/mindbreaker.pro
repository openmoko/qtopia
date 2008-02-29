CONFIG          += qtopiaapp

HEADERS		= mindbreaker.h
SOURCES		= mindbreaker.cpp main.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

TARGET          = mindbreaker


help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=mindbreaker.html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/mindbreaker.desktop
desktop.path=/apps/Games
pics.files=$${QTOPIA_DEPOT_PATH}/pics/mindbreaker/*
pics.path=/pics/mindbreaker
INSTALLS+=desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics
