qbuild {
SOURCEPATH+=ir
} else {
PREFIX=IR
VPATH+=ir
}

IR_HEADERS+=\
    qiriasdatabase.h\
    qirlocaldevice.h\
    qirnamespace.h\
    qirremotedevice.h \
    qirremotedevicewatcher.h \
    qirserver.h \
    qirsocket.h \
    qirglobal.h

IR_PRIVATE_HEADERS+=\
    qirnamespace_p.h \
    qirsocketengine_p.h

IR_SOURCES+=\
    qiriasdatabase.cpp\
    qirlocaldevice.cpp\
    qirnamespace.cpp\
    qirremotedevice.cpp \
    qirremotedevicewatcher.cpp \
    qirserver.cpp \
    qirsocket.cpp

IR_UNIX.TYPE=CONDITIONAL_SOURCES
IR_UNIX.CONDITION=unix
IR_UNIX.SOURCES=qirsocketengine_unix.cpp
!qbuild:IR_UNIX.PREFIX=IR
!qbuild:CONDITIONAL_SOURCES(IR_UNIX)

qbuild {
HEADERS+=$$IR_HEADERS
SOURCES+=$$IR_SOURCES
PRIVATE_HEADERS+=$$IR_PRIVATE_HEADERS
} else {
sdk_ir_headers.files=$$IR_HEADERS
sdk_ir_headers.path=/include/qtopia/comm
sdk_ir_headers.hint=sdk headers
INSTALLS+=sdk_ir_headers
}

