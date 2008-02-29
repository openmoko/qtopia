CONFIG		+= qtopiaapp

HEADERS		= videowidget.h \
                  videoplayer.h

SOURCES		= videowidget.cpp \
                  videoplayer.cpp \
                  main.cpp

TARGET		= videos

TRANSLATABLES   = $$HEADERS \
                    $$SOURCES 

DEPENDS         += mediaplayerbase
INCLUDEPATH     += ../../libraries/mediaplayer
DEPENDPATH     += ../../libraries/mediaplayer

# Need -lpthreads here to quieten valgrind
LIBS            += -lmediaplayer -lpthread

pics.files=$${QTOPIA_DEPOT_PATH}/pics/videos/*
pics.path=/pics/videos
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/videos.desktop
desktop.path=/apps/Applications
help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=videos*
service.files=$$QTOPIA_DEPOT_PATH/services/PlayMedia/videos
service.path=/services/PlayMedia
INSTALLS+=desktop service
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=Video Player for the Qtopia environment.
PACKAGE_DEPENDS=qpe-libmediaplayer

