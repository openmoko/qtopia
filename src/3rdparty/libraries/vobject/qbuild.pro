TEMPLATE=lib
CONFIG+=qtopia staticlib use_pic singleexec
TARGET=vobject

MODULE_NAME=vobject
LICENSE=FREEWARE

include(vobject.pro)

headers.TYPE=DEPENDS PERSISTED
headers.EVAL="INCLUDEPATH+="$$path(.,project)

