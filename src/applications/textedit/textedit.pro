qtopia_project(qtopia app)
TARGET=textedit
CONFIG+=qtopia_main

HEADERS		= textedit.h
SOURCES		= textedit.cpp main.cpp

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=textedit*
help.hint=help
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/textedit.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
pics.files=$$QTOPIA_DEPOT_PATH/pics/textedit/*
pics.path=/pics/textedit
pics.hint=pics
openservice.files=$$QTOPIA_DEPOT_PATH/services/Open/text/plain/textedit
openservice.path=/services/Open/text/plain
viewservice.files=$$QTOPIA_DEPOT_PATH/services/View/text/plain/textedit
viewservice.path=/services/View/text/plain
INSTALLS+=help desktop pics openservice viewservice

pkg.description=Text editor for Qtopia.
pkg.domain=window,cardreader,docapi{text/*},print
