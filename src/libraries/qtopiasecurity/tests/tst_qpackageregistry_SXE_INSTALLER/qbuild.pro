include(tst_qpackageregistry_SXE_INSTALLER.pro)
TEMPLATE=app
CONFIG+=qtopia unittest

SOURCEPATH+=/src/libraries/qtopiasecurity
SOURCEPATH+=/src/libraries/qtopiabase
requires(enable_sxe)


TARGET=tst_qpackageregistry_SXE_INSTALLER
