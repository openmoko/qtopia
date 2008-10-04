CONFIG-=qt
TEMPLATE=app
SOURCES=main.cpp
include(../locate_drmagent.pri)
LIBS+=$$DRMAGENT -lpthread
