# include() this .pri file in any unit test which wants to use
# the functionality in qfuturesignal.h

qbuild {
    PWD=$$path(.,existing)
}

SOURCES*=\
        $$PWD/qfuturesignal.cpp

HEADERS*=\
        $$PWD/qfuturesignal.h

INCLUDEPATH*=$$PWD

qbuild {
    MODULES*=qtopiabase
} else {
    depends(libraries/qtopiabase)
}

