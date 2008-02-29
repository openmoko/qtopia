TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= minimal.h ../hello/hello.h
SOURCES		= minimal.cpp \
		  ../hello/hello.cpp \
		  main.cpp
TARGET		= winmanager
DEPENDPATH=../../include
REQUIRES=full-config
