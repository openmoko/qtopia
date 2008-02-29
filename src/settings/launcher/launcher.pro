TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= launchersettings.h wallpaperselector.h colorselector.h
SOURCES		= main.cpp launchersettings.cpp wallpaperselector.cpp colorselector.cpp
INTERFACES	= launchersettingsbase.ui wallpaperselectorbase.ui
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe
TARGET		= launchersettings

