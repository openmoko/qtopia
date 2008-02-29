CONFIG		+= qtopiaapp

HEADERS		= minefield.h \
		  minesweep.h
SOURCES		= minefield.cpp \
		  minesweep.cpp \
		  main.cpp

TRANSLATABLES = $${SOURCES} $${HEADERS}
                  
TARGET		= minesweep


help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=minesweep.html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/minesweep.desktop
desktop.path=/apps/Games
pics.files=$${QTOPIA_DEPOT_PATH}/pics/minesweep/*
pics.path=/pics/minesweep
INSTALLS+=desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics
