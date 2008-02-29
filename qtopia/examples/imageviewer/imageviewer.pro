TEMPLATE   = app
CONFIG          += qtopia warn_on release
HEADERS		= showimage.h \
		  settingsdialog.h

SOURCES		= settingsdialog.cpp \
		  showimage.cpp main.cpp

INTERFACES	= settingsdialogbase.ui

TARGET          = showimage
