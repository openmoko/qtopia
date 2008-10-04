# This is an application
TEMPLATE=app

# The binary name
TARGET=example

# This app uses Qtopia
CONFIG+=qtopia

# Build this app as a quicklauncher plugin
# You need to be using the QTOPIA_ADD_APPLICATION/QTOPIA_MAIN macros or this will not work!
#CONFIG+=quicklaunch

# Build this project into the singleexec binary
#CONFIG+=singleexec

# Specify the languages that make lupdate should produce .ts files for
STRING_LANGUAGE=en_US
LANGUAGES=en_US de

# These are the source files that get built to create the application
FORMS=examplebase.ui
HEADERS=example.h
SOURCES=main.cpp example.cpp

# Install the launcher item. The metadata comes from the .desktop file
# and the path here.
desktop.files=example.desktop
desktop.path=/apps/Applications
desktop.trtarget=example-nct
desktop.hint=nct desktop

# Install some pictures.
pics.files=pics/*
pics.path=/pics/example
pics.hint=pics

# Install help files
help.source=help
help.files=example.html
help.hint=help

# SXE information
target.hint=sxe
target.domain=untrusted

# Package information (used for make packages)
pkg.name=example
pkg.desc=Example Application
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=Commercial
