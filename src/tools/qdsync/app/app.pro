qtopia_project(qtopia app)
TARGET=qdsync
CONFIG+=no_tr singleexec_main

#CONFIG+=qtopia_main no_tr no_quicklaunch
## Force no quicklaunch (because this is a daemon and it can start slow, unlike other things at startup time)
#CONFIG-=force_quicklaunch

VPATH+=..

SOURCES+=\
    main.cpp\

pkg.name=$$TARGET
pkg.desc=Synchronization
pkg.version=$$VERSION-1

# This is how I ask SXE for permission to do things.
# 'window' lets me have a GUI
# 'pim' is for the PIM code (plugins cannot give privileges to the apps that load them)
# 'qdsync' gives me LIDS access and allows some QCop messages to be sent inter-process
pkg.domain=window,pim,qdsync

desktop.files=../qdsync.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

help.source=../help
help.files=qdsync.html
help.hint=help
INSTALLS+=help

pics.files=../pics/*
pics.path=/pics/qdsync
pics.hint=pics
INSTALLS+=pics
