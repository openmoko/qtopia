!qbuild {
qtopia_project(qtopia app)
TARGET=mediaplayer
CONFIG+=qtopia_main
depends(libraries/qtopiamedia)
enable_pictureflow:depends(3rdparty/libraries/pictureflow)
}

enable_pictureflow:MODULES*=pictureflow

DEFINES+=NO_VISUALIZATION
#DEFINES+=NO_THUMBNAIL

HEADERS = mediaplayer.h \
            playercontrol.h \
            statewidget.h \
            elidedlabel.h \
            playerwidget.h \
            mediabrowser.h \
            visualization.h \
            browser.h \
            keyhold.h \
            menumodel.h \
            menuview.h \
            requesthandler.h \
            servicerequest.h \
            keyfilter.h \
            playmediaservice.h

SOURCES = main.cpp \
            mediaplayer.cpp \
            playercontrol.cpp \
            statewidget.cpp \
            elidedlabel.cpp \
            playerwidget.cpp \
            mediabrowser.cpp \
            visualization.cpp \
            browser.cpp \
            keyhold.cpp \
            menumodel.cpp \
            menuview.cpp \
            requesthandler.cpp \
            keyfilter.cpp \
            playmediaservice.cpp

!contains(QTOPIAMEDIA_ENGINES,helix) {
    DEFINES += NO_HELIX
}

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/mediaplayer.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

pics.files=$$QTOPIA_DEPOT_PATH/pics/mediaplayer/*
pics.path=/pics/mediaplayer
pics.hint=pics
INSTALLS+=pics

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=mediaplayer*
help.hint=help
INSTALLS+=help

playmediaservice.files = $$QTOPIA_DEPOT_PATH/services/PlayMedia/mediaplayer
playmediaservice.path = /services/PlayMedia
INSTALLS+=playmediaservice

pkg.desc=Qtopia media player.
pkg.domain=trusted
