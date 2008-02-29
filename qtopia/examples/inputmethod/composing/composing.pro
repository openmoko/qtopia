# can be built in two different ways.
# the first is as an application to demonstrate the composing nature of the 
# input method without bringing in complications to do with making it a plugin.
# the second method shows how it can then be turned into a plugin.

# coment the following line if you want to make the input method as an
# application
CONFIG += composeim_as_plugin

TARGET       = composeim

composeim_as_plugin {
    CONFIG      += qtopiaplugin
    HEADERS      = composeim.h composeimpl.h
    SOURCES      = composeim.cpp composeimpl.cpp
} else {
    CONFIG      += qtopiaapp
    CONFIG	-= buildQuicklaunch
    HEADERS      = composeim.h
    SOURCES      = composeim.cpp main.cpp
}

TRANSLATABLES=$$HEADERS $$SOURCES

composeim_as_plugin:QTOPIA_PROJECT_TYPE=inputmethods

