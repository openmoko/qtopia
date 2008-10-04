!qbuild{
qtopia_project(qtopia plugin)
TARGET=ericssonvendor
CONFIG+=no_tr
depends(libraries/qtopiaphonemodem)
}

HEADERS		=  vendor_ericsson_p.h ericssonplugin.h
SOURCES	        =  vendor_ericsson.cpp ericssonplugin.cpp

