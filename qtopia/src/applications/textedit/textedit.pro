CONFIG		+= qtopiaapp
HEADERS		= textedit.h
SOURCES		= textedit.cpp main.cpp

TARGET		= textedit

TRANSLATABLES   = $$HEADERS \
                    $$SOURCES \
                    inserttable.ui
                     

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=textedit*
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/textedit.desktop
desktop.path=/apps/Applications
pics.files=$${QTOPIA_DEPOT_PATH}/pics/textedit/*
pics.path=/pics/textedit
openservice.files=$$QTOPIA_DEPOT_PATH/services/Open/text/plain/textedit
openservice.path=/services/Open/text/plain
viewservice.files=$$QTOPIA_DEPOT_PATH/services/View/text/plain/textedit
viewservice.path=/services/View/text/plain
INSTALLS+=desktop openservice viewservice
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=The text editor for the Qtopia environment.
