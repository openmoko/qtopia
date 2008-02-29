requires(contains(QTOPIA_ARCH,sharp))

TEMPLATE	= app
CONFIG		= singleexec
DESTDIR		= $$(QPEDIR)/bin

SOURCES		= atd.cpp

TARGET		= atd

target.path=/bin
at.files=$$(QPEDIR)/bin/at
at.path=/bin
INSTALLS+=target at

TRANSLATIONS=

PACKAGE_NAME=atd
PACKAGE_DESCRIPTION=atd daemon for Sharp

