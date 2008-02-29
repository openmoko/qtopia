TEMPLATE	= app
CONFIG		= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= controlwidgetbase.h imageutil.h documentlist.h loopcontrol.h mediaplayerplugininterface.h playlistselection.h \
		  id3tag.h action.h fileinfo.h tabbedselector.h mediaplayerstate.h \
		  videowidget.h audiowidget.h playlistwidget.h mediaplayer.h audiodevice.h
SOURCES		= controlwidgetbase.cpp imageutil.cpp documentlist.cpp main.cpp \
		  loopcontrol.cpp playlistselection.cpp mediaplayerstate.cpp \
		  id3tag.cpp fileinfo.cpp tabbedselector.cpp \
		  videowidget.cpp audiowidget.cpp playlistwidget.cpp mediaplayer.cpp audiodevice.cpp
TARGET		= mpegplayer
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe -lpthread

# CONFIG+=static
# TMAKE_CXXFLAGS += -DQPIM_STANDALONE
# LIBS		+= libmpeg3/libmpeg3.a -lpthread

TRANSLATIONS	= $(QPEDIR)/i18n/de/mpegplayer.ts
