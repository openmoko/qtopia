!qbuild {
    qtopia_project(qtopia plugin)
    TARGET=voicemailviewer

    requires(enable_qtopiamedia)

    depends(libraries/qtopia)
    depends(libraries/qtopiamail)
    depends(libraries/qtopiapim)
    depends(libraries/homeui)
    depends(libraries/qtopiamedia)
}

HEADERS+=\
    voicemailviewer.h

SOURCES+=\
    voicemailviewer.cpp

