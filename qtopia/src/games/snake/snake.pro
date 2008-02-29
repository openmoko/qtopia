CONFIG		+= qtopiaapp

TARGET		= snake

HEADERS		= snake.h target.h obstacle.h interface.h codes.h border.h sprites.h
SOURCES		= snake.cpp target.cpp obstacle.cpp interface.cpp border.cpp main.cpp sprites.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=snake*

desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/snake.desktop
desktop.path=/apps/Games
pics.files=$${QTOPIA_DEPOT_PATH}/pics/snake/*
pics.path=/pics/snake
INSTALLS+=desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics
