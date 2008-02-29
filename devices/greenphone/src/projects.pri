PROJECTS*=\
    devtools\
    devtools/apm\
    devtools/qipkg-simple\
    devtools/chvol\
    devtools/startupflags\
    devtools/dbservices\
    plugins/qtopiacore/mousedrivers/greenphone\
    plugins/qtopiacore/kbddrivers/greenphone

!free_package|free_plus_binaries {
    for(p,PHONEVENDORS) {
        exists(plugins/phonevendors/$$p/$$tail($$p).pro):PROJECTS*=plugins/phonevendors/$$p
    }
    for(m,MULTIPLEXERS) {
        exists(plugins/multiplexers/$$m/$$tail($$m).pro):PROJECTS*=plugins/multiplexers/$$m
    }
}
