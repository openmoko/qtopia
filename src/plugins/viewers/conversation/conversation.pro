!qbuild {
    qtopia_project(qtopia plugin)
    TARGET=conversationviewer

    depends(libraries/qtopiamail)
    equals(QTOPIA_UI,home):depends(libraries/homeui)
}

HEADERS+=\
    conversationviewer.h\
    conversationdelegate.h

SOURCES+=\
    conversationviewer.cpp\
    conversationdelegate.cpp

