qtopia_project(qtopia app)
TARGET=fifteen
CONFIG+=qtopia_main no_quicklaunch

HEADERS		= fifteen.h
SOURCES		= fifteen.cpp main.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Games/fifteen.desktop
desktop.path=/apps/Games
desktop.hint=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=fifteen.html
help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/fifteen/*
pics.path=/pics/fifteen
pics.hint=pics
INSTALLS+=desktop help pics

pkg.desc=Try to get the fifteen pieces in order by sliding them around.
pkg.domain=window,games,docapi,cardreader
