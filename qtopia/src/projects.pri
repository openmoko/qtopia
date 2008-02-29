# This file performs marshalling of the various other .pri files
# It also sets up LIBRARY_PROJECTS properly (core libs go first)

!contains( processed_pri, projects.pri ) {
    processed_pri += projects.pri

    include($${QTOPIA_DEPOT_PATH}/src/general.pri)
    include($${QTOPIA_DEPOT_PATH}/src/commercial.pri)
    include($${QTOPIA_DEPOT_PATH}/src/custom.pri)
    include($$(QPEDIR)/src/local.pri)

    LIBRARY_PROJECTS=$$CORE_LIBRARY_PROJECTS $$LIBRARY_PROJECTS

    # dependencies
    # required for dependencies
    !contains(LIBRARY_PROJECTS,libraries/handwriting) {
	PLUGIN_PROJECTS-=plugins/inputmethods/handwriting\
		plugins/inputmethods/fshandwriting
	APP_PROJECTS -= settings/handwriting
    }

    !contains(LIBRARY_PROJECTS,libraries/qtopiapim) {
	LIBRARY_PROJECTS -= libraries/qtopiapim1
	APP_PROJECTS -= applications/addressbook\
		applications/datebook\
		applications/todo\
		applications/qtmail
    }

    !contains(LIBRARY_PROJECTS,libraries/mediaplayer) {
	PLUGIN_PROJECTS-=3rdparty/plugins/codecs/libmad \
		3rdparty/plugins/codecs/libffmpeg \
		3rdparty/plugins/codecs/libamr

	QTOPIA_PHONE:LIBRARY_PROJECTS-=libraries/qtopiasmil

	LIBRARY_PROJECTS -= 3rdparty/libraries/libavcodec \
		3rdparty/libraries/libavformat \
		3rdparty/libraries/amr

	APP_PROJECTS -= applications/videos \
		applications/music \
		applications/mediarecorder

	PLUGIN_PROJECTS-=plugins/codecs/wavplugin \
		plugins/codecs/wavrecord
    }

    !contains(APP_PROJECTS,applications/videos):!contains(APP_PROJECTS,applications/music) {
	THEME_PROJECTS-=mediaplayer/techno
    }

    !contains(LIBRARY_PROJECTS,libraries/qtopiacalc) {
	APP_PROJECTS-=applications/calculator

	PLUGIN_PROJECTS-=plugins/calculator/fraction\
	    plugins/calculator/conversion\
	    plugins/calculator/simple\
	    plugins/calculator/advanced
    }
    !contains(LIBRARY_PROJECTS,libraries/qtopiamail) {
	APP_PROJECTS-=applications/qtmail
	PLUGIN_PROJECTS-=plugins/today/email
# TODO also should check if no qtmail app, what libraries to drop.
    }
    !contains(APP_PROJECTS,applications/datebook) {
	PLUGIN_PROJECTS-=plugins/today/datebook
    }
    !contains(APP_PROJECTS,applications/todo) {
	PLUGIN_PROJECTS-=plugins/today/todo
    }
    !contains(APP_PROJECTS,applications/today) {
	PLUGIN_PROJECTS-=plugins/today/email\
	    plugins/today/datebook\
	    plugins/today/todo
    }
    # exclusive use dependencies
    # wavrecord/play gsm only needed if medierecorder built
    !contains(APP_PROJECTS,applications/mediarecorder) {
	PLUGIN_PROJECTS-=plugins/codecs/wavplugin plugins/codecs/wavrecord
	LIBRARY_PROJECTS-=3rdparty/libraries/gsm
    }

    # only used with ffmpeg/amr
    !contains(PLUGIN_PROJECTS,3rdparty/plugins/codecs/libffmpeg) {
	!contains(PLUGIN_PROJECTS,3rdparty/plugins/codecs/libamr):LIBRARY_PROJECTS-=3rdparty/libraries/amr
	LIBRARY_PROJECTS-=3rdparty/libraries/libavcodec\
	    3rdparty/libraries/libavformat
    }

    # Features stuff

    !contains(LIBRARY_PROJECTS,libraries/qtopiasmil) {
        FEATURES+=QTOPIA_NO_MMS
    }

    # qpe_features.h lets things rebuild that would normally require a make clean first
    system($${QTOPIA_DEPOT_PATH}/bin/setupfeatures $$(QPEDIR) $$FEATURES)

}

