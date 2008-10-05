include(tst_farenforcer.pro)
TEMPLATE=app
CONFIG+=qtopia unittest

SOURCEPATH+=/qtopiacore/qt/src/gui/embedded
MOC_COMPILE_EXCEPTIONS+=qtransportauth_qws_p.h

TARGET=tst_farenforcer
