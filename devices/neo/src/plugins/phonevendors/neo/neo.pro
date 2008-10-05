!qbuild{
qtopia_project(qtopia plugin)
TARGET=neovendor
CONFIG+=no_tr
depends(libraries/qtopiaphonemodem)
}

HEADERS		=  vendor_neo_p.h neoplugin.h
SOURCES	        =  vendor_neo.cpp neoplugin.cpp


