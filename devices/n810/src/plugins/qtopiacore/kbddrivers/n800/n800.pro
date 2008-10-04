!qbuild{
qtopia_project(embedded qtopia core plugin)
TARGET = n800kbdhandler
CONFIG+=no_tr
}

HEADERS = n800kbddriverplugin.h n800kbdhandler.h
SOURCES = n800kbddriverplugin.cpp n800kbdhandler.cpp

