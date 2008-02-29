CONFIG          += qtopiaapp

HEADERS		= mindbreaker.h
SOURCES		= mindbreaker.cpp main.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

TARGET          = mindbreaker


help.files=$${QTOPIA_DEPOT_PATH}/help/html/mindbreaker.html
help.path=/help/html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/mindbreaker.desktop
desktop.path=/apps/Games
pics.files=$${QTOPIA_DEPOT_PATH}/pics/mindbreaker/*
pics.path=/pics/mindbreaker
INSTALLS+=desktop help
PICS_INSTALLS+=pics
