!qbuild:qtopia_project(unittest)
HEADERS=openobexheaderprocessor.h \
        ../headertestdata.h
SOURCES=openobexheaderprocessor.cpp \
        ../headertestdata.cpp \
        tst_qobexheader_p.cpp
!qbuild:depends(libraries/qtopiacomm)
!qbuild:depends(3rdparty/libraries/openobex)

