TEMPLATE     = lib
CONFIG      += qt warn_on release
HEADERS      = composeim.h \
               composeimpl.h
SOURCES      = composeimpl.cpp \
               composeim.cpp
TARGET       = composeim
DESTDIR      = $(QPEDIR)/plugins/inputmethods
INCLUDEPATH += $(QPEDIR)/include
LIBS        += -lqpe
VERSION      = 1.0.0
