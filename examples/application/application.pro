qtopia_project(qtopia app) # see buildsystem.html for more project keywords
TARGET=example
CONFIG+=qtopia_main no_singleexec no_quicklaunch no_tr

FORMS=examplebase.ui
HEADERS=example.h
SOURCES=main.cpp example.cpp

desktop.files=example.desktop
desktop.path=/apps/Applications
desktop.trtarget=example-nct
desktop.hint=nct desktop

pics.files=pics/*
pics.path=/pics/example
pics.hint=pics

help.source=help
help.files=example.html
help.hint=help

INSTALLS+=desktop pics help

pkg.name=example
pkg.desc=Example Application
pkg.version=1.0.0-1
pkg.maintainer=Trolltech (www.trolltech.com)
pkg.license=Commercial
pkg.domain=window
