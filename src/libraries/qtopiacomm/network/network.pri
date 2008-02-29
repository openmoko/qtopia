PREFIX=NETWORK
VPATH+=network

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
    qtopiahttp.h

NETWORK_SOURCES+=\
    ipconfig.cpp\
    accountconfig.cpp\
    proxiesconfig.cpp\
    qnetworkstate.cpp\
    scriptthread.cpp\
    qnetworkdevice.cpp\
    ipvalidator.cpp\
    hexkeyvalidator.cpp\
    qtopiahttp.cpp

SSL_HEADERS=\
    qtsslsocket.h\
    ssllibrary.h
SSL_SOURCES=\
    qtsslsocket.cpp\
    ssllibrary.cpp
enable_ssl {
    depends(3rdparty/libraries/openssl/ssl)
    NETWORK_HEADERS+=$$SSL_HEADERS
    NETWORK_SOURCES+=$$SSL_SOURCES
}

sdk_network_headers.files=$$NETWORK_HEADERS
sdk_network_headers.path=/include/qtopia/comm
sdk_network_headers.hint=sdk headers
INSTALLS+=sdk_network_headers

