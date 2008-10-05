include(tst_qobexheader.pro)
TEMPLATE=app
CONFIG+=qtopia unittest


QTOPIA*=comm
MODULES*=openobex
get_sourcepath(qtopiacomm)

TARGET=tst_qobexheader
