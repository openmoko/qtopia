qtopia_project(qtopia plugin)
TARGET=genericviewer

!enable_modem:DEFINES+=QTOPIA_NO_SMS

HEADERS+=\
    attachmentoptions.h\
    browser.h\
    genericviewer.h

SOURCES+=\
    attachmentoptions.cpp\
    browser.cpp\
    genericviewer.cpp

TRANSLATABLES +=    $$HEADERS\
                    $$SOURCES

depends(libraries/qtopiamail)
