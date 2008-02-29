TEMPLATE	=   lib
CONFIG		+=  warn_on release
CONFIG		-= qt
HEADERS		=   gsm.h
SOURCES		=   add.c \
		    code.c \
		    decode.c \
		    gsm_create.c \
		    gsm_decode.c \
		    gsm_destroy.c \
		    gsm_encode.c \
		    gsm_option.c \
		    long_term.c \
		    lpc.c \
		    preprocess.c \
		    rpe.c \
		    short_term.c \
		    table.c
TARGET		=   gsm
DESTDIR		=   $(QPEDIR)/lib
INCLUDEPATH	+=  $(QPEDIR)/include
DEPENDPATH	+=  $(QPEDIR)/include
VERSION		=   1.0.0
DEFINES    +=  FAST SASR WAV49
