CONFIG		+= qtopiaapp

HEADERS		= audiowidget.h \
                  musicplayer.h

SOURCES		= audiowidget.cpp \
                  musicplayer.cpp \
                  main.cpp

TARGET		= music

TRANSLATABLES   = $$HEADERS \
                    $$SOURCES 

DEPENDS         += mediaplayerbase
INCLUDEPATH     += ../../libraries/mediaplayer
DEPENDPATH     += ../../libraries/mediaplayer

LIBS            += -lmediaplayer -lpthread

pics.files=$${QTOPIA_DEPOT_PATH}/pics/music/*
pics.path=/pics/music
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/music.desktop
desktop.path=/apps/Applications
help.files=$${QTOPIA_DEPOT_PATH}/help/html/music*
help.path=/help/html
INSTALLS+=desktop help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Music Player for the Qtopia environment.
PACKAGE_DEPENDS=qpe-libmediaplayer

