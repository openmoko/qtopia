#This file contains projects that make up the Qtopia Media module.

PROJECTS*=\
        libraries/qtopiaaudio

enable_qtopiamedia {
    PROJECTS*=\
            libraries/qtopiamedia \
            libraries/qtopiavideo \
            tools/mediaserver \
            server/media/servercontrol \                #(re)starts media server
            plugins/content/id3 \
            plugins/content/threegpp

    !equals(QTOPIA_UI,home) {
        PROJECTS+=applications/mediaplayer
    } else {
        PROJECTS+=applications/musicplayer
    }

    qws:PROJECTS*=\
            plugins/videooutput/directpaintervideooutput

    PROJECTS*=\
            server/ui/volumedlg \                       #shown when volume keys pressed
            server/media/volumemanagement \             #volume control backend
            server/ui/poweralert \                      #low power alert dialog
            server/media/alertservice                  #audible system alerts

    enable_phonon:PROJECTS*=plugins/phonon_backend/qtopiamedia

    !equals(QTOPIA_UI,home) { 
        PROJECTS*=\
                server/media/keyclick \                     #audible key pressed
                server/media/genericvolumeservice           #generic volume service

        qws:PROJECTS*=\
                server/media/screenclick                    #audible touch screen clicks

    }

    enable_pictureflow:PROJECTS*=3rdparty/libraries/pictureflow

    enable_qvfb {
        PROJECTS*=\
            plugins/audiohardware/desktop
    }

    contains(QTOPIAMEDIA_ENGINES,helix) {
        PROJECTS*=\
            3rdparty/libraries/helix \
            plugins/mediaengines/helix
    }

    contains(QTOPIAMEDIA_ENGINES,gstreamer) {
        PROJECTS*=\
            3rdparty/libraries/gstreamer \
            plugins/mediaengines/gstreamer
    }

    contains(QTOPIAMEDIA_ENGINES,cruxus) {
        PROJECTS*=plugins/mediaengines/cruxus
        # MIDI support
        PROJECTS*=\
            3rdparty/libraries/libtimidity\
            3rdparty/plugins/codecs/libtimidity
        # OGG Vorbis support
        PROJECTS*=\
            3rdparty/libraries/tremor\
            3rdparty/plugins/codecs/tremor
        # WAV support
        PROJECTS*=\
            plugins/codecs/wavplay
    }

} else {
    !x11:PROJECTS*=\
        tools/qss\ # The non-qtopiamedia mediaserver (supports QSound only)
        server/media/servercontrol \                #(re)starts media server
}

