include(tst_qpackageregistry.pro)
TEMPLATE=app
CONFIG+=qtopia unittest

SOURCEPATH+=/src/libraries/qtopiasecurity
requires(enable_sxe)

TARGET=tst_qpackageregistry
