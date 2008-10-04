qtopia_project(qtopia app)
TARGET=wordgame
requires(porting)

# main.cpp uses the QTOPIA_ADD_APPLICATION/QTOPIA_MAIN macros
# It can also build without these macros if you disable this and change the define in main.cpp
CONFIG+=qtopia_main
# Do not build this app into a singleexec binary
CONFIG+=no_singleexec
# Disable i18n support
CONFIG+=no_tr

FORMS	= newgamebase.ui rulesbase.ui
HEADERS		= wordgame.h
SOURCES		= wordgame.cpp main.cpp

help.source=help
help.files=wordgame.html
help.hint=help
INSTALLS+=help
desktop.files=wordgame.desktop
desktop.path=/apps/Games
desktop.hint=desktop
INSTALLS+=desktop
pics.files=pics/*
pics.path=/pics/wordgame
pics.hint=pics
INSTALLS+=pics

