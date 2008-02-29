TEMPLATE	=  lib
CONFIG		+= qtopia warn_on release
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		=  libflashplugin.h   libflashpluginimpl.h
SOURCES		=  libflashplugin.cpp libflashpluginimpl.cpp \
		   adpcm.cc character.cc flash.cc graphic16.cc matrix.cc script.cc \
		   sprite.cc bitmap.cc cxform.cc font.cc graphic24.cc movie.cc \
		   shape.cc sqrt.cc button.cc displaylist.cc graphic.cc graphic32.cc \
		   program.cc sound.cc text.cc
TARGET		=  flashplugin
DESTDIR		=  $(QPEDIR)/plugins/codecs
VERSION		=  1.0.0

TRANSLATIONS = libflashplugin-en_GB.ts libflashplugin-de.ts libflashplugin-ja.ts libflashplugin-no.ts
