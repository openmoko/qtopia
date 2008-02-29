CONFIG		+= qtopiaapp
HEADERS		= textedit.h
SOURCES		= textedit.cpp main.cpp

TARGET		= textedit

TRANSLATABLES   = $$HEADERS \
                    $$SOURCES \
                    inserttable.ui
                     

help.files=$${QTOPIA_DEPOT_PATH}/help/html/textedit*
help.path=/help/html

helppics.files=$${QTOPIA_DEPOT_PATH}/help/html/textedit*
helppics.path=/help/html
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/textedit.desktop
desktop.path=/apps/Applications
pics.files=$${QTOPIA_DEPOT_PATH}/pics/textedit/*
pics.path=/pics/textedit
INSTALLS+=desktop help
PICS_INSTALLS+=pics helppics

PACKAGE_DESCRIPTION=The text editor for the Qtopia environment.
