CONFIG          += qtopiaapp

HEADERS		= d3des.h \
		  krfbbuffer.h \
		  krfbcanvas.h \
		  krfbconnection.h \
		  krfbdecoder.h \
		  krfblogin.h \
		  krfboptions.h \
		  krfbserverinfo.h \
		  kvnc.h \
		  kvncconnectdlg.h \
		  kvncoptionsdlg.h \
		  vncauth.h

SOURCES		= d3des.c \
		  vncauth.c \
		  krfbbuffer.cpp \
		  krfbcanvas.cpp \
		  krfbconnection.cpp \
		  krfbdecoder.cpp \
		  krfblogin.cpp \
		  krfboptions.cpp \
		  kvnc.cpp \
		  kvncconnectdlg.cpp \
		  kvncoptionsdlg.cpp \
		  main.cpp
		  
help.files=$${QTOPIA_DEPOT_PATH}/help/html/keypebble*
help.path=/help/html
pics.files=$${QTOPIA_DEPOT_PATH}/pics/keypebble/*
pics.path=/pics/keypebble
INSTALLS+=help
PICS_INSTALLS+=pics

INTERFACES	= vncoptionsbase.ui

TRANSLATABLES = $${HEADERS} \
                $${SOURCES} \
                $${INTERFACES} 

TARGET          = keypebble

