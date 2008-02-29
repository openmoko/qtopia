TEMPLATE	=   lib
CONFIG		-= qt
CONFIG          -= warn_on
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
DESTDIR		=   $$(QPEDIR)/lib
VERSION		=   1.0.0
DEFINES    +=  FAST SASR WAV49
PACKAGE_NAME=gsm

TRANSLATIONS=
