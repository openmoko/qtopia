CONFIG		+= qtopiaapp
HEADERS		= helpbrowser.h \
		    helppreprocessor.h
SOURCES		= helpbrowser.cpp main.cpp \
		    helppreprocessor.cpp

TARGET		= helpbrowser

TRANSLATABLES   = $$HEADERS \
                    $$SOURCES

help.files=$${QTOPIA_DEPOT_PATH}/help/html/helpbrowser*
help.path=/help/html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/helpbrowser.desktop
desktop.path=/apps/Applications
# See $QPEDIR/src/server/server.pro for the definition of extra html help files
#    not associted with a specific application. Each application is responsible 
#    for installing its own help
pics.files=$${QTOPIA_DEPOT_PATH}/pics/helpbrowser/*
pics.path=/pics/helpbrowser
pics2.files=$${QTOPIA_DEPOT_PATH}/pics/help/*
pics2.path=/pics/help
INSTALLS+=desktop help
PICS_INSTALLS+=pics pics2

PACKAGE_DESCRIPTION=The HTML help browser for the Qtopia environment.
