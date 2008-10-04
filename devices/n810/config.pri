equals(QTOPIA_UI,home) {
    # Home edition projects
    DEFINES+=QTOPIA_HOMEUI_WIDE
}

# common

#DEFINES+=DEBUG
DEFINES+=QT_QWS_N810
DEFINES+=HAVE_V4L2
# n810 has smaller than normal alsa ring buffer, writing out more than
# the buffer size causes audio to stop working all together, requiring reboot.
# MIN_ALSA_BUFFER changes QAudioOutput::write to only write out a max of buffer size
# to alsa interface.
DEFINES+=MIN_ALSA_BUFFER

LAN_NETWORK_CONFIGS=wlan lan
