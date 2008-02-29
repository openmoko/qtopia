# can be built in two different ways.
# the first is as an application to demonstrate the 
# input method without bringing in complications to do with making it a plugin.
# the second method shows how it can then be turned into a plugin.

# coment the following line if you want to make the input method as an
# application
CONFIG += popupim_as_plugin

# comment the following line if you want to make the input method as
# and older style InputMethodInterface, compatible with Qtopia 1.5
CONFIG += popupim_extended_interface

TARGET       = popupim

popupim_as_plugin {
    CONFIG      += qtopiaplugin
    popupim_extended_interface {
	HEADERS      = popupim.h popupextimpl.h
	SOURCES      = popupim.cpp popupextimpl.cpp
    } else {
	HEADERS      = popupim.h popupimpl.h
	SOURCES      = popupim.cpp popupimpl.cpp
    }
} else {
    CONFIG      += qtopiaapp
    CONFIG	-= buildQuicklaunch
    HEADERS      = popupim.h
    SOURCES      = popupim.cpp main.cpp
}

TRANSLATABLES=$$HEADERS $$SOURCES

popupim_as_plugin:QTOPIA_PROJECT_TYPE=inputmethods

