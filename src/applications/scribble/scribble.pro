CONFIG	    += qtopiaapp

HEADERS	    = colorselector.h scribblepad.h scribbler.h scribble.h
SOURCES	    = colorselector.cpp scribblepad.cpp scribbler.cpp \
		scribble.cpp main.cpp

TRANSLATABLES = $$HEADERS \
                    $$SOURCES
                
TARGET	    = scribble
