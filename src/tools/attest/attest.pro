qtopia_project(qtopia app)
TARGET=attest
CONFIG+=no_tr singleexec_main

depends(libraries/qtopiacomm)

SOURCES = attest.cpp
