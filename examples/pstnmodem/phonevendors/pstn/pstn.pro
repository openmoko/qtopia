qtopia_project(qtopia plugin)
requires(enable_cell)
TARGET=pstnvendor

CONFIG+=no_tr

HEADERS		=  vendor_pstn_p.h pstnplugin.h
SOURCES	        =  vendor_pstn.cpp pstnplugin.cpp

depends(libraries/qtopiaphonemodem)

