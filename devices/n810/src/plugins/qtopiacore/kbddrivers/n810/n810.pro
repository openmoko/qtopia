!qbuild{
qtopia_project(embedded qtopia core plugin)
TARGET = n810kbdhandler
CONFIG+=no_tr
}

HEADERS = n810kbddriverplugin.h n810kbdhandler.h
SOURCES = n810kbddriverplugin.cpp n810kbdhandler.cpp

