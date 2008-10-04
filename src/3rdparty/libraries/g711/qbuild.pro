TEMPLATE=lib
CONFIG+=qtopia staticlib use_pic singleexec
TARGET=g711

MODULE_NAME=g711
LICENSE=FREEWARE

include(g711.pro)

headers.TYPE=DEPENDS PERSISTED
headers.EVAL="INCLUDEPATH+="$$path(.,project)

