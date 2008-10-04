!qbuild{
qtopia_project(desktop lib)
TARGET=phonesim
CONFIG+=no_tr no_singleexec

depends(libraries/qtopia,fake)
depends(libraries/qtopiacomm,fake)
depends(libraries/qtopiaphone,fake)
depends(libraries/qtopiabase,fake)

VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiaphone
VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiacomm
VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase

INCLUDEPATH+=$$QPEDIR/include/qtopiacomm
INCLUDEPATH+=$$QPEDIR/include/qtopiaphone
INCLUDEPATH+=$$QPEDIR/include/qtopiabase
INCLUDEPATH+=$$QPEDIR/include/qtopia
}

DEFINES+=PHONESIM
HEADERS= phonesim.h server.h hardwaremanipulator.h \
                  qsmsmessagelist.h \
                  qsmsmessage.h \
                  qcbsmessage.h \
                  callmanager.h \
                  simfilesystem.h \
                  simapplication.h \
                  serial/qgsmcodec.h \
                  serial/qatutils.h \
                  serial/qatresultparser.h \
                  serial/qatresult.h \
		  wap/qwsppdu.h \
                  qsimcommand.h \
                  qsimenvelope.h \
                  qsimterminalresponse.h \
                  qsimcontrolevent.h\
                  qtopialog.h
SOURCES= phonesim.cpp server.cpp hardwaremanipulator.cpp \
                  qsmsmessagelist.cpp \
		  qsmsmessage.cpp \
		  qcbsmessage.cpp \
                  callmanager.cpp \
                  simfilesystem.cpp \
                  simapplication.cpp \
                  serial/qgsmcodec.cpp \
                  serial/qatutils.cpp \
                  serial/qatresultparser.cpp \
                  serial/qatresult.cpp \
		  wap/qwsppdu.cpp \
                  qsimcommand.cpp \
                  qsimenvelope.cpp \
                  qsimterminalresponse.cpp \
                  qsimcontrolevent.cpp\
                  qtopialog.cpp

!qbuild{
headers.files=$$HEADERS
headers.hint=headers
headers.path=/include/phonesim
INSTALLS+=headers

qt_inc(phonesim)
idep(LIBS+=-l$$TARGET)
}
