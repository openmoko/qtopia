# To build the examples, run configure -build $QPEDIR/examples
TEMPLATE	= subdirs
CONFIG		+= ordered

SUBDIRS		= application
!QTOPIA_PHONE:SUBDIRS+=inputmethod

