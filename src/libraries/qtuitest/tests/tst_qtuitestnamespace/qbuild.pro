include(tst_qtuitestnamespace.pro)
TEMPLATE=app
CONFIG+=qtopia unittest
SOURCEPATH+= /src/libraries/qtuitest
TARGET=tst_qtuitestnamespace
MOC_COMPILE_EXCEPTIONS+=qtuitestwidgets_p.h
