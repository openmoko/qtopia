include(tst_qsimtoolkit.pro)
TEMPLATE=app
CONFIG+=qtopia unittest
include(/tests/shared/qfuturesignal.pri)

SOURCEPATH+=/src/server/phone/ui/components/simapp

QTOPIA*=phone

TARGET=tst_qsimtoolkit
