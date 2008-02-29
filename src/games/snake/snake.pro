qtopia_project(qtopia app)
TARGET=snake
CONFIG+=qtopia_main no_quicklaunch

HEADERS	=	snakeview.h \
		snakescene_p.h \
		snakemanager_p.h  \
		wall_p.h \
		brick_p.h \
		messagebox_p.h \
		mouse_p.h \
		snake_p.h \
		snakeiteminterface_p.h \
		snakeitem_p.h

SOURCES =	snakeview.cpp \
		main.cpp \
		snakescene_p.cpp \
		snakemanager_p.cpp  \
		wall_p.cpp \
		brick_p.cpp \
		messagebox_p.cpp \
		mouse_p.cpp \
		snake_p.cpp \
		snakeitem_p.cpp
		
#HEADERS		= snake.h interface.h sprites.h
#SOURCES		= snake.cpp interface.cpp main.cpp sprites.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=snake*
help.hint=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Games/snake.desktop
desktop.path=/apps/Games
desktop.hint=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/snake/*
pics.path=/pics/snake
pics.hint=pics
INSTALLS+=help desktop pics

pkg.desc=Try to keep the snake alive for as long as possible by eating mice and avoiding walls.
pkg.domain=window,games
