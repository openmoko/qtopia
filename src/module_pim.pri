#This file contains projects that make up the PIM module.

# Addressbook, datebook & todo
PROJECTS*=\
    libraries/qtopiamail\
    3rdparty/libraries/vobject \
    libraries/qtopiapim \
    applications/addressbook

PROJECTS*=\
    server/pim/savetocontacts \                 #simple msg box to save contacts
    server/pim/servercontactmodel \             #customized contact model for server
    server/pim/buddysyncer                      #presence synchronization

enable_cell{      
    PROJECTS*=server/pim/simcard                                  #SIM Card phonebook sync
}

# Voice notes/recorder
PROJECTS*=\
    libraries/qtopiaaudio 



!equals(QTOPIA_UI,home) {
    PROJECTS*=\
        applications/datebook \
        applications/todolist \
        tools/qdsync/pim \
        plugins/codecs/wavrecord \      #mediarecorder requires this
        applications/mediarecorder \

    # Low-level synchronization support
    PROJECTS*=\
        server/processctrl/qdsync                  #(re)starts qdsync process
    PROJECTS*=\
        tools/qdsync/app \
        tools/qdsync/base
    qbuild:PROJECTS*=tools/qdsync/common/qdsync_common
    else:PROJECTS*=tools/qdsync/common
}
