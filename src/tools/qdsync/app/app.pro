!qbuild{
qtopia_project(qtopia app)
TARGET=qdsync
# Omit qtopia_main, thus forcing this app to not be quicklaunched.  This is 
# because this is a daemon and it can get away with starting slow, unlike
# other things at startup time.
CONFIG+=no_tr singleexec_main
VPATH+=..
}

SOURCES+=\
    main.cpp\

pkg.desc=Synchronization
pkg.domain=trusted
pkg.multi=\
    tools/qdsync/base\
    tools/qdsync/common\
    tools/qdsync/pim

desktop.files=../qdsync.desktop
desktop.path=/apps/Applications
desktop.hint=nct desktop
desktop.trtarget=qdsync-nct
INSTALLS+=desktop

help.source=../help
help.files=\
    qdsync.html\
    usbgadget-serial-qdsync.html
help.hint=help
INSTALLS+=help

pics.files=../pics/*
pics.path=/pics/qdsync
pics.hint=pics
INSTALLS+=pics

