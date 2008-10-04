!qbuild{
qtopia_project(qtopia core app)
TARGET=qtopia-dbus-launcher
CONFIG+=no_tr
depends(3rdparty/libraries/qtdbus)
}

HEADERS		= launcher.h
SOURCES		= main.cpp launcher.cpp 

pkg.desc=Interprocess communication client for Qtopia.
pkg.domain=trusted
