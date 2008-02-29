qtopia_project(stub)

# Fonts...
!isEmpty(QTOPIA_FONTS) {
    fonts.files=$$QTOPIA_FONTS
    fonts.path=/lib/fonts
    INSTALLS+=fonts

    fontdir.commands=$$COMMAND_HEADER\
        :>$(INSTALL_ROOT)/lib/fonts/fontdir
    fontdir.path=/lib/fonts
    INSTALLS+=fontdir
} else {
    font_sources=$$QTE_DEPOT_PATH/dist/embedded/lib/fonts $$QTOPIA_DEPOT_PATH/dist/fonts
    font_styles=$$QTOPIA_FONT_STYLES
    contains(QTOPIA_DISP_ROTS,0):font_rotations+=$$font_styles
    contains(QTOPIA_DISP_ROTS,90):for(w,font_styles):font_rotations+=$${w}_t5
    contains(QTOPIA_DISP_ROTS,180):for(w,font_styles):font_rotations+=$${w}_t15
    contains(QTOPIA_DISP_ROTS,270):for(w,font_styles):font_rotations+=$${w}_t10
    font_sizes=$$QTOPIA_FONT_SIZES
    contains(font_sizes,all):font_sizes=*
    font_families=$$QTOPIA_FONT_FAMILIES
    for(f,font_families) {
        for(s,font_sizes) {
            for(r,font_rotations) {
                for(font_source,font_sources) {
                    font_files+=$$font_source/$${f}_$${s}_$${r}.qpf
                }
            }
        }
    }
    
    # Create a "font compiler" to handle the wildcards (should we bother with combine?)
    font_installer.CONFIG=no_link no_dependencies no_build ignore_no_exist no_clean
    font_installer.commands=$$COMMAND_HEADER\
        mkdir -p $(INSTALL_ROOT)$$libdir/fonts $$LINE_SEP_VERBOSE\
        install -c ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
    font_installer.input=font_files
    font_installer.output=$(INSTALL_ROOT)$$libdir/fonts/${QMAKE_FILE_BASE}.qpf
    font_installer.name=font_installer
    QMAKE_EXTRA_COMPILERS+=font_installer
    
    for(font_source,font_sources) {
        fontdir.files+=$$font_source/fontdir
    }
    fontdir.path=/lib/fonts
    fontdir.depends=compiler_font_installer_make_all
    INSTALLS+=fontdir
}

pkg.name=qt-embedded
pkg.desc=Qt/Embedded
pkg.version=$$QTE_VERSION
pkg.domain=libs

# package descriptions for the fonts (based on the .control files that were here)
#for(r,font_rotations) {
#    font_files=
#    for(f,font_families) {
#        for(s,font_sizes) {
#            font_files+=$$font_source/$${f}_$${s}_$${r}.qpf
#        }
#    }
#    eval(font_$${r}.name=qt-embedded-fonts-\$$r)
#    eval(font_$${r}.desc=Qt/Embedded fonts for rotation \$$r)
#    eval(font_$${r}.files=\$$font_files)
#    PACKAGES+=font_$$r
#}

!disable_qt_lupdate {
    #
    # We want to maintain our own translations of Qt.
    #
    # This could be done more simply with a find command
    # but that would not be cross-platform.
    #

    # Only do the libs we have
    QCLIBS=
    for(p,PROJECTS) {
        containstext($$p,libraries/qtopiacore/):QCLIBS+=$$tail($$p)
    }
    QTLIBS=
    for(p,PROJECTS) {
        # Don't do libQt3Support because we don't use it
        containstext($$p,libraries/qt/):!containstext($$p,qt3support):QTLIBS+=$$tail($$p)
    }

    debug=$$(QMAKE_DEBUG_ON)
    exists($$QPEDIR/src/build/debug_on):debug=1
    equals(debug,1) {
        message(Qtopia Core projects:)
        for(f,QCLIBS):message(- $$f)
        message(Qt projects:)
        for(f,QTLIBS):message(- $$f)
    }

    # We need to recurse directories so a function is used
    defineTest(getAllTranslatables) {
        for(cwd,TO_PROCESS) {
            contains(PROCESSED_DIRS,$$cwd):next()
            echo(Searching $$cwd)
            PROCESSED_DIRS+=$$cwd

            FILES=$$files($$cwd/*)
            for(f,FILES) {
                containsre($$f,\.(cpp|h|ui)$):TRANSLATABLES*=$$fixpath($$f)
                # Check for directories (ie. something with more files in it)
                subfiles=$$files($$f/*)
                !isEmpty(subfiles) {
                    TO_PROCESS*=$$fixpath($$f)
                }
            }
        }

        export(PROCESSED_DIRS)
        export(TO_PROCESS)
        export(TRANSLATABLES)
    }

    # Since QTE_DEPOT_PATH and DQT_DEPOT_PATH should be the same use *= so we only do each dir once.
    # If QTE_DEPOT_PATH and DQT_DEPOT_PATH are different, then two sets of translations will be
    # stored in the one .ts file.
    PROCESSED_DIRS=
    TO_PROCESS=
    for(dir,QCLIBS) {
        TO_PROCESS*=$$fixpath($$QTE_DEPOT_PATH/src/$$dir)
    }
    for(dir,QTLIBS) {
        TO_PROCESS*=$$fixpath($$DQT_DEPOT_PATH/src/$$dir)
    }

    TRANSLATABLES=
    getAllTranslatables()
    for(l,forever) {
        equals(PROCESSED_DIRS,$$TO_PROCESS):break()
        getAllTranslatables()
    }

    equals(debug,1) {
        message(Translatables:)
        for(f,TRANSLATABLES):message(- $$f)
    }

    # We have TRANSLATABLES now so we can enable the i18n feature (it does lupdate/lrelease for us)
    !isEmpty(TRANSLATIONS):CONFIG*=i18n
    CONFIG-=no_tr
    TRTARGET=qt
}

