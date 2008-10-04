!qbuild{
qtopia_project(qtopia plugin)
TARGET=greenphonevendor
CONFIG+=no_tr
requires(enable_cell)
depends(libraries/qtopiaphonemodem)
}

HEADERS		=  vendor_greenphone_p.h greenphoneplugin.h
SOURCES	        =  vendor_greenphone.cpp greenphoneplugin.cpp

