TEMPLATE=app
CONFIG+=qtopia singleexec
TARGET=atinterface

QTOPIA*=phone comm
enable_cell:QTOPIA*=mail pim
QTOPIA.CONFIG=singleexec

include(atinterface.pro)

