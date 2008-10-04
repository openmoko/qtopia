!qbuild{
qtopia_project(embedded qtopia core plugin)
CONFIG+=no_tr
TARGET  = qtopiapiciconengine
depends(libraries/qtopiabase)
}

HEADERS += piciconengine.h
SOURCES += piciconengine.cpp

