TEMPLATE = lib
TARGET = ilbc
DESTDIR  = $$(QPEDIR)/lib
CONFIG-=qt

DEFINES += HAVE_CONFIG_H
HEADERS += anaFilter.h \
constants.h \
createCB.h \
doCPLC.h \
enhancer.h \
filter.h \
FrameClassify.h \
gainquant.h \
getCBvec.h \
helpfun.h \
hpInput.h \
hpOutput.h \
iCBConstruct.h \
iCBSearch.h \
iLBC_decode.h \
iLBC_define.h \
iLBC_encode.h \
LPCdecode.h \
LPCencode.h \
lsf.h \
packing.h \
StateConstructW.h \
StateSearchW.h \
syntFilter.h 


SOURCES += anaFilter.cpp \
constants.cpp \
createCB.cpp \
doCPLC.cpp \
enhancer.cpp \
filter.cpp \
FrameClassify.cpp \
gainquant.cpp \
getCBvec.cpp \
helpfun.cpp \
hpInput.cpp \
hpOutput.cpp \
iCBConstruct.cpp \
iCBSearch.cpp \
iLBC_decode.cpp \
iLBC_encode.cpp \
#iLBC_test.cpp \
LPCdecode.cpp \
LPCencode.cpp \
lsf.cpp \
packing.cpp \
StateConstructW.cpp \
StateSearchW.cpp \
syntFilter.cpp 

TRANSLATIONS =
