!qbuild{
qtopia_project(desktop app)
TARGET=pngscale
CONFIG+=no_install no_tr
mac:CONFIG	-= app_bundle
win32:LIBS	+= ole32.lib advapi32.lib
}

SOURCES		= main.cpp


