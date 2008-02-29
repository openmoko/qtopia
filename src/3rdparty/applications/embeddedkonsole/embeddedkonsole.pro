TEMPLATE	= app

CONFIG		+= qtopia warn_on release

DESTDIR		= $(QPEDIR)/bin

HEADERS		= TEWidget.h \
		  TEScreen.h \
		  TECommon.h \
		  TEHistory.h \
		  TEmulation.h \
		  TEmuVt102.h \
		  session.h \
		  keytrans.h \
		  konsole.h \
		  MyPty.h

SOURCES		= TEScreen.cpp \
		  TEWidget.cpp \
		  TEHistory.cpp \
		  TEmulation.cpp \
		  TEmuVt102.cpp \
		  session.cpp \
		  keytrans.cpp \
		  konsole.cpp \
		  main.cpp 

unix:SOURCES  += MyPty.cpp 
win32:SOURCES +=  \
                  ../../../server/qprocess.cpp \
                  ../../../server/qprocess_win.cpp 

win32:HEADERS += ../../../server/qprocess.h 

TARGET		= embeddedkonsole

REQUIRES	= embeddedkonsole

TRANSLATIONS = embeddedkonsole-en_GB.ts embeddedkonsole-de.ts embeddedkonsole-ja.ts embeddedkonsole-no.ts
