TEMPLATE	= app

CONFIG		+= qtopiaapp

# comment the following line to enable building the example application 
# as a quicklaunch application.
# quicklaunch applications need to be installed before they can be run,
# But can be faster to start running in Qtopia.
CONFIG		-= buildQuicklaunch

HEADERS		= example.h
SOURCES		= main.cpp example.cpp
INTERFACES	= examplebase.ui
TARGET		= example

TARGET.path = /bin

desktop.files = example.desktop
desktop.path = /apps/Applications
INSTALLS += desktop

pics.files=Example.png
pics.path=/pics/example
INSTALLS+=pics

help.files=example.html
help.path=/help/html
INSTALLS+=help

TRANSLATABLES = $$HEADERS $$SOURCES $$INTERFACES
NON_CODE_TRANSLATABLES = $$desktop.files
NON_CODE_TRTARGETS = example-nct
