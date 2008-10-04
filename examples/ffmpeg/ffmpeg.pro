!qbuild{
qtopia_project(qtopia plugin)
TARGET = ffmpeg
CONFIG += no_tr
requires(enable_ffmpeg)
depends(libraries/qtopiamedia)
depends(libraries/qtopiaaudio)
depends(libraries/qtopiavideo)
}

AVAILABLE_LANGUAGES=en_US
LANGUAGES=$$AVAILABLE_LANGUAGES

HEADERS	= \
        ffmpegengine.h \
        ffmpegengineinformation.h \
        ffmpegenginefactory.h \
        ffmpegurisessionbuilder.h \
        ffmpegplaybinsession.h \
        ffmpegdirectpainterwidget.h \
        ffmpegsinkwidget.h 

SOURCES	= \
        ffmpegengine.cpp \
        ffmpegengineinformation.cpp \
        ffmpegenginefactory.cpp \
        ffmpegurisessionbuilder.cpp \
        ffmpegplaybinsession.cpp \
        ffmpegdirectpainterwidget.cpp \
        ffmpegsinkwidget.cpp 

LIBS += -lavcodec -lavformat

ffmpeg_settings.files=ffmpeg.conf
ffmpeg_settings.path=/etc/default/Trolltech
INSTALLS += ffmpeg_settings
