!qbuild:qtopia_project( unittest )

SOURCES += tst_sxemonitor.cpp \
           ../../../../../src/tools/sxemonitor/sxemonitor.cpp \
           ../../../../../src/tools/sxemonitor/sxemonqlog.cpp

HEADERS += ../../../../../src/tools/sxemonitor/sxemonitor.h \
           ../../../../../src/tools/sxemonitor/sxemonqlog.h     

!qbuild:depends(libraries/qtopia)
