qtopia_project(qtopia core lib)
license(LGPL)
TARGET		=   dissipate2
VERSION		=   1.0.0
CONFIG+=no_tr
# Disable this in singleexec because it can't possibly work without violating the LGPL
idep(CONFIG+=no_singleexec,CONFIG)

HEADERS		=   \
    base64.h \
    extern_md5.h \
    headerparser.h \
    messagesocket.h \
    mimecontenttype.h \
    parameterlist.h \
    sdp.h \
    sipcall.h \
    sipclient.h \
    sipdebug.h \
    sipheader.h \
    sipmessage.h \
    sipprotocol.h \
    sipregister.h \
    sipstatus.h \
    siptransaction.h \
    sipuri.h \
    sipurilist.h \
    sipuser.h \
    siputil.h \
    sipvia.h \
    sipvialist.h \
    tcpmessagesocket.h \
    udpmessagesocket.h
SOURCES		=   \
    base64.c \
    extern_md5.c \
    headerparser.cpp \
    messagesocket.cpp \
    mimecontenttype.cpp \
    parameterlist.cpp \
    sdp.cpp \
    sipcall.cpp \
    sipclient.cpp \
    sipdebug.cpp \
    sipheader.cpp \
    sipmessage.cpp \
    sipprotocol.cpp \
    sipregister.cpp \
    sipstatus.cpp \
    siptransaction.cpp \
    sipuri.cpp \
    sipurilist.cpp \
    sipuser.cpp \
    siputil.cpp \
    sipvia.cpp \
    sipvialist.cpp \
    tcpmessagesocket.cpp \
    udpmessagesocket.cpp

DEFINES    +=  
LIBS       += -lresolv

pkg.desc=SIP library
pkg.domain=trusted

# FIXME "make syncqtopia"
dep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
