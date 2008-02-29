CONFIG		+= qtopiaapp

HEADERS		= minefield.h \
		  minesweep.h
SOURCES		= minefield.cpp \
		  minesweep.cpp \
		  main.cpp

TRANSLATABLES = $${SOURCES} $${HEADERS}
                  
TARGET		= minesweep


help.files=$${QTOPIA_DEPOT_PATH}/help/html/minesweep.html
help.path=/help/html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/minesweep.desktop
desktop.path=/apps/Games
pics.files=$${QTOPIA_DEPOT_PATH}/pics/minesweep/*
pics.path=/pics/minesweep
INSTALLS+=desktop help
PICS_INSTALLS+=pics
