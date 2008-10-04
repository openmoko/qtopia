
# common

PROJECTS*=\
    plugins/qtopiacore/kbddrivers/n810\
    plugins/audiohardware/nokia\
    plugins/qtopiacore/gfxdrivers/nokia\
    plugins/whereabouts/n810 \
    devtools\
    devtools/logread

!enable_qvfb {
    PROJECTS*=\
        plugins/cameras/v4lwebcams
}

equals(QTOPIA_UI,home) {
   # Home edition projects
 
    PROJECTS*=\
        server/media/genericvolumeservice

} else {
   # Phone edition projects


}

