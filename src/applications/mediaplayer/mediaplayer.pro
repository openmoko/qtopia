qtopia_project(qtopia app)
TARGET=mediaplayer
CONFIG+=qtopia_main

HEADERS = mediaplayer.h \
            playercontrol.h \
            statewidget.h \
            elidedlabel.h \
            playerwidget.h \
            mediabrowser.h \
            visualization.h \
            playlist.h

SOURCES = main.cpp \
            mediaplayer.cpp \
            playercontrol.cpp \
            statewidget.cpp \
            elidedlabel.cpp \
            playerwidget.cpp \
            mediabrowser.cpp \
            visualization.cpp \
            playlist.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/mediaplayer.desktop
desktop.path=/apps/Applications
desktop.hint=desktop

pics.files=$$QTOPIA_DEPOT_PATH/pics/mediaplayer/*
pics.path=/pics/mediaplayer
pics.hint=pics

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=mediaplayer*
help.hint=help

INSTALLS+=desktop pics help

pkg.desc=Qtopia media player.
pkg.domain=mediasession,window,graphics,docapi,launcher,drm,cardreader,lightandpower

depends(libraries/qtopiamedia)
