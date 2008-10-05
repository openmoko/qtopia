!qbuild:qtopia_project( unittest )

!qbuild:VPATH += $$QT_DEPOT_PATH/src/gui/embedded

SOURCES += tst_farenforcer.cpp \
           qtransportauth_qws.cpp \
           qwscommand_qws.cpp

HEADERS += qtransportauth_qws_p.h

