qtopia_project(qtopia plugin)
TARGET		=  amrrecordplugin

HEADERS		=  amrrecord.h amrrecordimpl.h
SOURCES	        =  amrrecord.cpp amrrecordimpl.cpp

depends(3rdparty/libraries/amr)
