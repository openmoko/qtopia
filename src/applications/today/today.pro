TEMPLATE = app
CONFIG += qtopia warn_on release
DESTDIR = $(QPEDIR)/bin

# Input
HEADERS += today.h todayoptions.h 
SOURCES += today.cpp todayoptions.cpp main.cpp

INCLUDEPATH += $(QPEDIR)/include \
		library
DEPENDPATH += $(QPEDIR)/include \
		library

LIBS += -lqpe -lqtopiaservices -Wl,-export-dynamic

INTERFACES = todayoptionsbase.ui

TARGET = today
