!qbuild{
qtopia_project(embedded app)
TARGET=phonesim_target
CONFIG+=no_tr no_install no_singleexec
VPATH += ../phonesim
VPATH += ../phonesim/lib

VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiaphone
VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiacomm
VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase

qt_inc(qtopiacomm)
depends(libraries/qtopiacomm,fake)
qt_inc(qtopiaphone)
depends(libraries/qtopiaphone,fake)
qt_inc(qtopiabase)
depends(libraries/qtopiabase,fake)
qt_inc(qtopia)
depends(libraries/qtopia,fake)
qt_inc(phonesim)
depends(tools/phonesim/lib,fake)
}

DEFINES += PHONESIM PHONESIM_TARGET

# This stops qtopiaipcmarshal.h including the D-BUS marshalling stubs,
# even though technically we are not building a host binary.
DEFINES += QTOPIA_HOST

HEADERS		= phonesim.h server.h simfilesystem.h callmanager.h \
                  serial/qgsmcodec.h serial/qatutils.h \
                  serial/qatresultparser.h serial/qatresult.h \
                  simapplication.h qsimcommand.h qsimterminalresponse.h \
                  qsimenvelope.h qsimcontrolevent.h\
                  qtopialog.h

SOURCES		= main.cpp phonesim.cpp server.cpp callmanager.cpp \
                  simfilesystem.cpp \
                  serial/qgsmcodec.cpp serial/qatutils.cpp \
                  serial/qatresultparser.cpp serial/qatresult.cpp \
                  simapplication.cpp qsimcommand.cpp qsimterminalresponse.cpp \
                  qsimenvelope.cpp qsimcontrolevent.cpp\
                  qtopialog.cpp

