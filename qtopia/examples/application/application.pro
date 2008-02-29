TEMPLATE	= app
CONFIG		+= qtopiaapp

# comment the following line to enable building the example application 
# as a quicklaunch application.
CONFIG		-= buildQuicklaunch

# Set this to 1.5 to target the Sharp Zaurus.
# Set this to 1.7 to target the Archos PMA400.
# Set this to 2 to target a Qtopia 2.x device.
# Leave this blank to use highest value the SDK's allows.
# This prevents linking against libraries that aren't
# present in older Qtopia releases. You will need to avoid
# new functions in old classes too. You can target a
# newer version and install on an older device if you also
# copy the extra libraries to the device.
QTOPIA_COMPAT_VER=1.5

TARGET		= example

PACKAGE_NAME=example
PACKAGE_DESCRIPTION=Example Application
PACKAGE_VERSION=1.0.0-1
PACKAGE_MAINTAINER=Trolltech (www.trolltech.com)
PACKAGE_LICENSE=GPL

HEADERS		= example.h
SOURCES		= main.cpp example.cpp
INTERFACES	= examplebase.ui

TARGET.path = /bin

desktop.files = example.desktop
desktop.path = /apps/Applications
INSTALLS += desktop

pics.files=pics/*
pics.path=/pics/example
PICS_INSTALLS+=pics

help.source=help
help.files=example.html
HELP_INSTALLS+=help

TRANSLATABLES = $$HEADERS $$SOURCES $$INTERFACES
NON_CODE_TRANSLATABLES = $$desktop.files
NON_CODE_TRTARGETS = example-nct
