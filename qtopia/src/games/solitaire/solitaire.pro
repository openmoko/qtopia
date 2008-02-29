CONFIG		+= qtopiaapp

HEADERS		= canvasitemtimer.h \
		cardmetrics.h \
		canvascard.h \
		canvasshapes.h \
		cardgame.h \
		cardpile.h \
		canvascardpile.h \
		card.h \
		canvascardgame.h \
		freecellcardgame.h \
		patiencecardgame.h \
		canvascardwindow.h

SOURCES		= canvasitemtimer.cpp \
		cardmetrics.cpp \
		canvascard.cpp \
		canvasshapes.cpp \
		cardgame.cpp \
		cardpile.cpp \
		canvascardpile.cpp \
		card.cpp \
		canvascardgame.cpp \
		freecellcardgame.cpp \
		patiencecardgame.cpp \
		canvascardwindow.cpp \
		main.cpp

QTOPIA_PHONE:INTERFACES	= phone_settings.ui
!QTOPIA_PHONE:INTERFACES = pda_settings.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} \
                    phone_settings.ui pda_settings.ui

TARGET		= solitaire


help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=solitaire*
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Games/solitaire.desktop
desktop.path=/apps/Games
pics.files=$${QTOPIA_DEPOT_PATH}/pics/solitaire/*
pics.path=/pics/solitaire
cardpics.files=$${QTOPIA_DEPOT_PATH}/pics/cards/*.png $${QTOPIA_DEPOT_PATH}/pics/cards/icons $${QTOPIA_DEPOT_PATH}/pics/cards/i18n
contains(QTOPIA_DISP_SIZES,C):cardpics.files+=$${QTOPIA_DEPOT_PATH}/pics/cards/micro
contains(QTOPIA_DISP_SIZES,P):cardpics.files+=$${QTOPIA_DEPOT_PATH}/pics/cards/small
contains(QTOPIA_DISP_SIZES,W)|contains(QTOPIA_DISP_SIZES,T):cardpics.files+=$${QTOPIA_DEPOT_PATH}/pics/cards/normal
contains(QTOPIA_DISP_SIZES,T):cardpics.files+=$${QTOPIA_DEPOT_PATH}/pics/cards/large
cardpics.path=/pics/cards
INSTALLS+=desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics cardpics
