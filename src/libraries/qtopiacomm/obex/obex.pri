PREFIX=OBEX
VPATH+=obex

OBEX_HEADERS+=\
    qobexpushclient.h\
    qobexpushservice.h\
    qobexsocket.h\
    qobexserver.h\
    qobexnamespace.h

OBEX_SOURCES+=\
    qobexpushclient.cpp\
    qobexpushservice.cpp\
    qobexsocket.cpp\
    qobexserver.cpp\
    qobexcommand_p.cpp\
    qobexcommon_p.cpp\
    qobexnamespace.cpp

OBEX_PRIVATE_HEADERS+=\
    qobexcommon_p.h\
    qobexcommand_p.h

sdk_obex_headers.files=$$OBEX_HEADERS
sdk_obex_headers.path=/include/qtopia/comm
sdk_obex_headers.hint=sdk headers
INSTALLS+=sdk_obex_headers

sdk_obex_private_headers.files=$$OBEX_PRIVATE_HEADERS
sdk_obex_private_headers.path=/include/qtopiacomm/private
sdk_obex_private_headers.hint=sdk headers
INSTALLS+=sdk_obex_private_headers

