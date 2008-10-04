!qbuild{
qtopia_project(embedded qtopia core plugin)
TARGET=homestyle
depends(libraries/homeui)
depends(libraries/qtopia)
depends(libraries/qtopiagfx)
}

HEADERS		= homestyle.h
SOURCES		= homestyle.cpp

