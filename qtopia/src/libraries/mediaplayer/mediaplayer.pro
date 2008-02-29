CONFIG		+= qtopia

TEMPLATE        = lib 

HEADERS += action.h audiodevice.h controlwidgetbase.h fileinfo.h \
           framebuffer.h id3tag.h imageutil.h mediaplayerstate.h \
           documentlist.h mediaselector.h mediaselectorwidget.h \
           mediaplayer.h videooutput.h loadingwidget.h ticker.h \
           loopcontrol.h loopcontrolbase.h loopcontextbase.h \
           audiocontext.h videocontext.h videoviewer.h mutex.h\
           mediaselectorbase.h

SOURCES += controlwidgetbase.cpp fileinfo.cpp framebuffer.cpp \
           id3tag.cpp imageutil.cpp mediaplayerstate.cpp \
           documentlist.cpp mediaselector.cpp mediaselectorwidget.cpp \
           mediaplayer.cpp videooutput.cpp loadingwidget.cpp ticker.cpp \
           loopcontrol.cpp loopcontextbase.cpp videoviewer.cpp
    
QT_QSS_MANAGED_DSP:SOURCES+=audiodevice_oss.cpp
!QT_QSS_MANAGED_DSP:SOURCES+=audiodevice_qss.cpp

# LIBS		+=  -L/usr/local/lib/valgrind
LIBS            += -lpthread

TRANSLATABLES   = $${HEADERS} \
                    $${SOURCES}
                  
TARGET		= mediaplayer

DESTDIR         = $$(QPEDIR)/lib

# DEFINES	+= _GNU_SOURCE
# INCLUDEPATH	+=  $${QTOPIA_DEPOT_PATH}/src/libraries/threads
# LIBS          +=  -lthreads

QTOPIA_PHONE:skin.files=$${QTOPIA_DEPOT_PATH}/pics/mediaplayer/skins/phone/*
QTOPIA_PHONE:skin.path=/pics/mediaplayer/skins/phone
!QTOPIA_PHONE:skin.files=$${QTOPIA_DEPOT_PATH}/pics/mediaplayer/skins/default/*
!QTOPIA_PHONE:skin.path=/pics/mediaplayer/skins/default
pics.files=$${QTOPIA_DEPOT_PATH}/pics/mediaplayer/icons
pics.path=/pics/mediaplayer
PICS_INSTALLS+=skin pics

plugindir.files=$${QTOPIA_DEPOT_PATH}/plugins/codecs/.directory
plugindir.path=/plugins/codecs/

INSTALLS+=plugindir

# handled in src/ipk_groups.pri
