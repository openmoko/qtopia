!qbuild{
qtopia_project(embedded qtopia core plugin)
CONFIG+=no_tr
TARGET  = qtopiasvgiconengine
}

HEADERS += svgiconengine.h
SOURCES += svgiconengine.cpp

