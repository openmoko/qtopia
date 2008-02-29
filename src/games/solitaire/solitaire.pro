multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= canvasitemtimer.h   cardmetrics.h   canvascard.h   canvasshapes.h   cardgame.h   cardgamelayout.h   cardpile.h   card.h   carddeck.h   canvascardgame.h   freecellcardgame.h   patiencecardgame.h   canvascardwindow.h
SOURCES		= canvasitemtimer.cpp cardmetrics.cpp canvascard.cpp canvasshapes.cpp cardgame.cpp cardgamelayout.cpp cardpile.cpp card.cpp carddeck.cpp canvascardgame.cpp freecellcardgame.cpp patiencecardgame.cpp canvascardwindow.cpp 

multiprocess:SOURCES+=main.cpp

TARGET		= patience

TRANSLATIONS = patience-en_GB.ts patience-de.ts patience-ja.ts patience-no.ts
