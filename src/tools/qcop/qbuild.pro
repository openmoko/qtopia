TEMPLATE=app
CONFIG+=qtopia singleexec
TARGET=qcop

MODULES*=qtopia::headers
QTOPIA.CONFIG=singleexec

include(qcop.pro)

