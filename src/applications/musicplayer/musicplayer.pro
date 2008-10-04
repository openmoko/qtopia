!qbuild {
qtopia_project(qtopia app)
TARGET=musicplayer
CONFIG+=qtopia_main
depends(libraries/qtopiamedia)
depends(libraries/homeui)
depends(3rdparty/libraries/pictureflow)
}

MODULES*=pictureflow

HEADERS = musicplayer.h \
          albumview.h \
          sidebar.h

SOURCES = main.cpp \
          musicplayer.cpp \
          albumview.cpp \
          sidebar.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/musicplayer.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

pics.files=$$QTOPIA_DEPOT_PATH/pics/mediaplayer/*
pics.path=/pics/mediaplayer
pics.hint=pics
INSTALLS+=pics

service.files=$$QTOPIA_DEPOT_PATH/src/applications/musicplayer/service/musicplayer
service.path=/services/MusicPlayer
INSTALLS+=service

pkg.desc=Qtopia music player.
pkg.domain=trusted
