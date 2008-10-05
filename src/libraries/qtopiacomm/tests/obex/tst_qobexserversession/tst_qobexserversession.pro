!qbuild:qtopia_project(unittest)
HEADERS=putclientwithnofirstpacketbody.h
SOURCES=putclientwithnofirstpacketbody.cpp\
        tst_qobexserversession.cpp

!qbuild:depends(libraries/qtopiacomm)
!qbuild:depends(3rdparty/libraries/openobex)
