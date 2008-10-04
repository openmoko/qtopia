!qbuild{
qtopia_project(qtopia plugin)
TARGET=inputmethodsitem
CONFIG+=no_tr
depends(server/core_server)
depends(libraries/qtopiatheming)
}

HEADERS =   inputmethodsitem.h

SOURCES =   inputmethodsitem.cpp
            
