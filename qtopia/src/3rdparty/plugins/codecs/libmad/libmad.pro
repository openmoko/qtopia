CONFIG		+=  qtopiaplugin

HEADERS		=   libmad_version.h fixed.h bit.h timer.h stream.h frame.h synth.h decoder.h \
		    layer12.h layer3.h huffman.h libmad_global.h mad.h libmadplugin.h libmadpluginimpl.h
SOURCES	        =   version.c fixed.c bit.c timer.c stream.c frame.c synth.c decoder.c \
		    layer12.c layer3.c huffman.c libmadplugin.cpp libmadpluginimpl.cpp

TARGET		=   madplugin

LIBS		+= -lpthread

contains(QMAKE_ARCH,x86_) | contains(QMAKE_ARCH,mmx_) {
    DEFINES	    +=	FPM_INTEL
} else {
    contains(QMAKE_ARCH,arm) {
	DEFINES	    +=	FPM_ARM
    } else {
	DEFINES	    +=	FPM_64BIT
    }
}

