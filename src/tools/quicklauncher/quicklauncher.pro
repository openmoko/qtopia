qtopia_project(qtopia app)
TARGET=quicklauncher
CONFIG+=no_tr
CONFIG+=singleexec_main

requires(!equals(LAUNCH_METHOD,normal))
depends(libraries/qtopiapim)

equals(LAUNCH_METHOD,quicklaunch) {
    HEADERS+=quicklaunch.h
    SOURCES+=quicklaunch.cpp
} else {
    HEADERS+=quicklaunchforked.h
    SOURCES+=quicklaunchforked.cpp
    DEFINES+=QUICKLAUNCHER_FORKED
}

SOURCES+=main.cpp

pkg.desc=Quicklauncher stub for quicklaunch enabled applications.
pkg.domain=quicklauncher,cardreader,prefix
