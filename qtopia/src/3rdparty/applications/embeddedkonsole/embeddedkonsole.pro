CONFIG		+= qtopiaapp

HEADERS		= TEWidget.h \
		  TEScreen.h \
		  TECommon.h \
		  TEHistory.h \
		  TEmulation.h \
		  TEmuVt102.h \
		  session.h \
		  keytrans.h \
		  konsole.h \
		  MyPty.h

SOURCES		= TEScreen.cpp \
		  TEWidget.cpp \
		  TEHistory.cpp \
		  TEmulation.cpp \
		  TEmuVt102.cpp \
		  session.cpp \
		  keytrans.cpp \
		  konsole.cpp \
		  main.cpp

TRANSLATABLES += $${HEADERS} \
                    $${SOURCES} \
                    MyPty.cpp

unix:SOURCES  += MyPty.cpp 


TARGET		= embeddedkonsole

pics.files=$${QTOPIA_DEPOT_PATH}/pics/embeddedkonsole/*
pics.path=/pics/embeddedkonsole
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/embeddedkonsole.desktop
desktop.path=/apps/Applications
keytabs.file=$${QTOPIA_DEPOT_PATH}/etc/keytabs
keytabs.path=/etc
INSTALLS+=desktop keytabs
PICS_INSTALLS+=pics

contains(QMAKE_ARCH,x86)|contains(QMAKE_ARCH,generic)|contains(QMAKE_ARCH,ipaq)|contains(QMAKE_ARCH,sharp) {
    LIBS+=-lutil
    DEFINES+=HAVE_OPENPTY
}
