!qbuild {
    qtopia_project(qtopia plugin)
    TARGET=genericviewer

    depends(libraries/qtopiamail)
    equals(QTOPIA_UI,home):depends(libraries/homeui)
}

!enable_cell {
    DEFINES+=QTOPIA_NO_SMS
    !enable_voip:DEFINES+=QTOPIA_NO_DIAL_FUNCTION
}

HEADERS+=\
    attachmentoptions.h\
    browser.h\
    genericviewer.h

SOURCES+=\
    attachmentoptions.cpp\
    browser.cpp\
    genericviewer.cpp

