CONFIG		+= qtopiaapp

HEADERS		= ledmeter.h sprites.h toplevel.h view.h
SOURCES		= ledmeter.cpp toplevel.cpp view.cpp main.cpp

TARGET		= qasteroids

TRANSLATABLES = $${HEADERS} $${SOURCES}

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=qasteroids*
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/qasteroids.desktop
desktop.path=/apps/Games
pics.files=$${QTOPIA_DEPOT_PATH}/pics/qasteroids/*
pics.path=/pics/qasteroids
sounds.files=$${QTOPIA_DEPOT_PATH}/sounds/qasteroids/*
sounds.path=/sounds/qasteroids
INSTALLS+=desktop sounds
HELP_INSTALLS+=help
PICS_INSTALLS+=pics
