TEMPLATE	= app
CONFIG		+= qtopia
TARGET		= qtopia-update-symlinks
DESTDIR		= $$(QPEDIR)/bin

HEADERS		= 
SOURCES		= main.cpp
INTERFACES	= 

target.path=/bin
INSTALLS+=target

TRANSLATIONS=
