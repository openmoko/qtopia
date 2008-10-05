PROJECTS*=\
    plugins/audiohardware/neo\
  	plugins/whereabouts/neo\
	  devtools

!enable_x11 {
	PROJECTS*= plugins/qtopiacore/kbddrivers/neo
}
	 
!qbuild:enable_cell {
    for(p,PHONEVENDORS) {
        exists(plugins/phonevendors/$$p/$$tail($$p).pro):PROJECTS*=plugins/phonevendors/$$p
    }
    for(m,MULTIPLEXERS) {
        exists(plugins/multiplexers/$$m/$$tail($$m).pro):PROJECTS*=plugins/multiplexers/$$m
    }
}


