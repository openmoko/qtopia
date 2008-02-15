THEMES=finxi
PROJECTS-=\
    games/qasteroids\
    settings/beaming\
    plugins/inputmethods/keyboard\
    plugins/inputmethods/dockedkeyboard\
    3rdparty/plugins/inputmethods/pkim\
    applications/camera

conf.files=$$PWD/etc/default/Trolltech/PredictiveKeyboard.conf
conf.path=/etc/default/Trolltech
INSTALLS+=conf
PROJECTS+=\
	../examples/scribble\
	tools/phonebounce


