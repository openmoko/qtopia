TEMPLATE    = app
CONFIG	    = qtopia warn_on release
DESTDIR	    = $(QPEDIR)/bin

HEADERS	    = buttoneditordialog.h \
	      ../../server/devicebutton.h \
	      ../../server/devicebuttonmanager.h
SOURCES	    = main.cpp \
	      buttoneditordialog.cpp \
	      ../../server/devicebutton.cpp \
              ../../server/devicebuttonmanager.cpp

INTERFACES  = buttonrow.ui
INCLUDEPATH += $(QPEDIR)/include
DEPENDPATH  += $(QPEDIR)/include
LIBS	    += -lqtopiaservices -lqpe

TARGET	    = buttoneditor
