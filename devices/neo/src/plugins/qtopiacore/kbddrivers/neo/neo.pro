!qbuild{
qtopia_project(embedded qtopia core plugin)
TARGET = neokbdhandler
CONFIG+=no_tr
}

HEADERS = neokbddriverplugin.h neokbdhandler.h
SOURCES = neokbddriverplugin.cpp neokbdhandler.cpp

