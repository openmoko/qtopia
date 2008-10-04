!qbuild{
qtopia_project(qtopia app)
TARGET=quicklauncher
CONFIG+=no_tr
CONFIG+=singleexec_main

requires(!equals(LAUNCH_METHOD,normal))
contains(QTOPIA_MODULES,pim):depends(libraries/qtopiapim)
}

HEADERS+=quicklaunch.h
SOURCES+=quicklaunch.cpp

contains(arch,mips) {
    # Temporary hack - prctl check doesn't seem to work.
    DEFINES+=QTOPIA_SETPROC_PRCTL
} else {
    equals(QTOPIA_SETPROC_METHOD,prctl):DEFINES+=QTOPIA_SETPROC_PRCTL
    equals(QTOPIA_SETPROC_METHOD,argv0):DEFINES+=QTOPIA_SETPROC_ARGV0
}

SOURCES+=main.cpp

pkg.desc=Quicklauncher stub for quicklaunch enabled applications.
pkg.domain=trusted
