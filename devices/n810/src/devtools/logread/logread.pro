!qbuild {
qtopia_project(external app)
TARGET=logread
CONFIG-=enable_singleexec
# Link with gcc not g++ so that binary is not linked with libstdc++.
QMAKE_LINK = $$QMAKE_CC
}

SOURCES	= logread.c

pkg.desc=Log reader to replace missing binary on N810
