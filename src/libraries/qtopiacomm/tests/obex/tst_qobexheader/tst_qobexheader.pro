!qbuild:qtopia_project(unittest)
HEADERS=../headertestdata.h
SOURCES=tst_qobexheader.cpp \
        ../headertestdata.cpp
!qbuild:depends(libraries/qtopiacomm)
!qbuild:depends(3rdparty/libraries/openobex)

