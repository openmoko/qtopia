TEMPLATE	= app

TARGET		= quicklauncher
DESTDIR         = $$(QPEDIR)/bin

CONFIG		+= qtopialib
SOURCES		= main.cpp

target.path	= /bin
INSTALLS	+= target

PACKAGE_DESCRIPTION=Quicklauncher stub for quicklaunch enabled applications.
PACKAGE_DEPENDS=qpe-libqtopia2

TRANSLATIONS=
