multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= maindocumentwidgetstack.h controlwidgetbase.h imageutil.h documentlist.h loopcontrol.h playlistselection.h \
		  framebuffer.h id3tag.h action.h fileinfo.h tabbedselector.h mediaplayerstate.h \
		  videowidget.h audiowidget.h playlistwidget.h mediaplayer.h audiodevice.h
SOURCES		= maindocumentwidgetstack.cpp controlwidgetbase.cpp imageutil.cpp documentlist.cpp \
		  framebuffer.cpp loopcontrol.cpp playlistselection.cpp mediaplayerstate.cpp \
		  id3tag.cpp fileinfo.cpp tabbedselector.cpp \
		  videowidget.cpp audiowidget.cpp playlistwidget.cpp mediaplayer.cpp audiodevice.cpp
TARGET		= mpegplayer

multiprocess:SOURCES += main.cpp

unix:LIBS       += -lpthread

TRANSLATIONS = mpegplayer-en_GB.ts mpegplayer-de.ts mpegplayer-ja.ts mpegplayer-no.ts
