TEMPLATE = lib
TARGET = dissipate2
INCLUDEPATH += $$QTOPIA_DEPOT_PATH/src/3rdparty/libraries/openssl/include
DESTDIR  = $$(QPEDIR)/lib
LIBS+=-lssl -lresolv

SOURCES += headerparser.cpp \
messagesocket.cpp \
mimecontenttype.cpp \
parameterlist.cpp \
sdp.cpp \
sipcall.cpp \
sipclient.cpp \
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
udpmessagesocket.cpp \
    base64.c \
    extern_md5.c

HEADERS += base64.h \
extern_md5.h \
headerparser.h \
messagesocket.h \
mimecontenttype.h \
parameterlist.h \
sdp.h \
sipcall.h \
sipclient.h \
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

TRANSLATABLES = $${HEADERS} $${SOURCES}
