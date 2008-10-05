qbuild {
SOURCEPATH+=network
} else {
PREFIX=NETWORK
VPATH+=network
}

NETWORK_FORMS+=\
    proxiesconfigbase.ui

NETWORK_HEADERS+=\
    ipconfig.h\
    accountconfig.h\
    proxiesconfig.h\
    qnetworkstate.h\
    scriptthread.h\
    qnetworkdevice.h\
    ipvalidator.h \
    hexkeyvalidator.h\
    qtopiahttp.h\
    qwapaccount.h\
    qwlanregistration.h\
    qnetworkconnection.h

# QBuild can't see that foo.cpp does #include "moc_foo.cpp"
MOC_COMPILE_EXCEPTIONS+=\
    qnetworkconnection.h\
    accountconfig.h

NETWORK_SOURCES+=\
    ipconfig.cpp\
    accountconfig.cpp\
    proxiesconfig.cpp\
    qnetworkstate.cpp\
    scriptthread.cpp\
    qnetworkdevice.cpp\
    ipvalidator.cpp\
    hexkeyvalidator.cpp\
    qtopiahttp.cpp\
    qwapaccount.cpp\
    qwlanregistration.cpp\
    qnetworkconnection.cpp

qbuild {
FORMS+=$$NETWORK_FORMS
HEADERS+=$$NETWORK_HEADERS
SOURCES+=$$NETWORK_SOURCES
} else {
sdk_network_headers.files=$$NETWORK_HEADERS
sdk_network_headers.path=/include/qtopia/comm
sdk_network_headers.hint=sdk headers
INSTALLS+=sdk_network_headers
}

