qtopia_project(qtopia app)
TARGET=helpbrowser
CONFIG+=qtopia_main

HEADERS		= helpbrowser.h \
		    helppreprocessor.h \
		navigationbar_p.h \
		bookmarksui.h \
                bookmark.h \
		bookmarkmodel.h \
		bookmarkdelegate.h \
		bookmarklist.h
SOURCES		= helpbrowser.cpp main.cpp \
		    helppreprocessor.cpp \
		navigationbar_p.cpp \
		bookmarksui.cpp \
                bookmark.cpp \
		bookmarkmodel.cpp \
		bookmarkdelegate.cpp \
		bookmarklist.cpp

# See $QTOPIA_SOURCE_TREE/src/server/server.pro for the definition of extra html help files
#    not associted with a specific application. Each application is responsible 
#    for installing its own help
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=helpbrowser*
help.hint=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/helpbrowser.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/helpbrowser/*
pics.path=/pics/helpbrowser
pics.hint=pics
pics2.files=$$QTOPIA_DEPOT_PATH/pics/help/*
pics2.path=/pics/help
pics2.hint=pics
helpservice.files=$$QTOPIA_DEPOT_PATH/services/Help/helpbrowser
helpservice.path=/services/Help
INSTALLS+=help desktop pics pics2 helpservice

pkg.desc=Help browser for Qtopia.
pkg.domain=window
