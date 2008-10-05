
HEADERS+=firstuse.h
SOURCES+=firstuse.cpp

dynamic.TYPE=CONDITIONAL_SOURCES
dynamic.CONDITION=!enable_singleexec
dynamic.HEADERS+=\
        ../../../../settings/systemtime/settime.h\
        ../../../../settings/language/languagesettings.h
dynamic.FORMS=\
    ../../../../settings/language/languagesettingsbase.ui
dynamic.SOURCES=\
    ../../../../settings/language/language.cpp\
    ../../../../settings/language/langmodel.cpp\
    ../../../../settings/systemtime/settime.cpp
!qbuild:CONDITIONAL_SOURCES(dynamic)
!qbuild:enable_singleexec:INCLUDEPATH+=$$OUT_PWD/../../../../settings/language/.ui/$$RELEASE_BIN_TYPE

qbuild:enable_singleexec {
    HEADERS+=\
        ../../../../settings/systemtime/settime.h\
        ../../../../settings/language/languagesettings.h
    MOC_COMPILE_EXCEPTIONS+=\
        ../../../../settings/systemtime/settime.h\
        ../../../../settings/language/languagesettings.h
}

