include(tst_qtuitestwidgets.pro)
TEMPLATE=app
CONFIG+=qtopia unittest
SOURCEPATH+= /src/libraries/qtuitest
TARGET=tst_qtuitestwidgets
MOC_COMPILE_EXCEPTIONS+=qtuitestwidgets_p.h
