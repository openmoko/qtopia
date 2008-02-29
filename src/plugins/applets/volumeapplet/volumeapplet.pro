TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= volume.h volumeappletimpl.h
SOURCES		= volume.cpp volumeappletimpl.cpp
TARGET		= volumeapplet
DESTDIR		= $(QPEDIR)/plugins/applets
VERSION		= 1.0.0

TRANSLATIONS = libvolumeapplet-en_GB.ts libvolumeapplet-de.ts libvolumeapplet-ja.ts libvolumeapplet-no.ts
