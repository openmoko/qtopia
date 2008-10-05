qtopia_project(qtopia app)
TARGET=qpe

include(main.pri)

# We want an en_US .directory file and dictionaries, even if en_US was disabled via -languages
commands=$$COMMAND_HEADER
INST_LANGUAGES=$$LANGUAGES
INST_LANGUAGES*=en_US
for(lang,INST_LANGUAGES) {
    !equals(commands,$$COMMAND_HEADER):commands+=$$LINE_SEP
    commands+=\
        mkdir -p $(INSTALL_ROOT)$$resdir/i18n/$$lang $$LINE_SEP_VERBOSE\
        install -c $$QTOPIA_DEPOT_PATH/i18n/$$lang/.directory $(INSTALL_ROOT)$$resdir/i18n/$$lang
}
langfiles.commands=$$commands
langfiles.CONFIG=no_path
INSTALLS+=langfiles

qdawggen=$$QPEDIR/bin/qdawggen
!equals(QTOPIA_HOST_ENDIAN,$$QTOPIA_TARGET_ENDIAN):qdawggen+=-e
commands=$$COMMAND_HEADER
for(lang,INST_LANGUAGES) {
    exists($$QTOPIA_DEPOT_PATH/etc/dict/$$lang/words) {
        !equals(commands,$$COMMAND_HEADER):commands+=$$LINE_SEP
        commands+=\
            mkdir -p $(INSTALL_ROOT)$$resdir/etc/dict/$$lang $$LINE_SEP\
            find $$QTOPIA_DEPOT_PATH/etc/dict/$$lang -maxdepth 1 -type f | xargs -r $$qdawggen $(INSTALL_ROOT)$$resdir/etc/dict/$$lang
    }
}
dicts.commands=$$commands
dicts.CONFIG=no_path
INSTALLS+=dicts

ALL_TRANSLATIONS=$$AVAILABLE_LANGUAGES
ALL_TRANSLATIONS-=$$STRING_LANGUAGE

# lupdate for "global" stuff
nct_lupdate.commands=$$COMMAND_HEADER\
    cd $$PWD;\
    find $$QTOPIA_DEPOT_PATH\
        # apps, plugins and i18n
        $${LITERAL_BACKSLASH}( $${LITERAL_BACKSLASH}( -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/apps/*$${LITERAL_SQUOTE} -o\
              -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/plugins/*$${LITERAL_SQUOTE} -o\
              -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/i18n/*$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) -a\
           # .directory and .desktop files
           $${LITERAL_BACKSLASH}( -name $${LITERAL_SQUOTE}.directory$${LITERAL_SQUOTE} -o -name $${LITERAL_SQUOTE}*.desktop$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) $${LITERAL_BACKSLASH}) -o\
        # etc
        $${LITERAL_BACKSLASH}( $${LITERAL_BACKSLASH}( -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/etc/*$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) -a\
           # .conf and .scheme files
           $${LITERAL_BACKSLASH}( -name $${LITERAL_SQUOTE}*.conf$${LITERAL_SQUOTE} -o -name $${LITERAL_SQUOTE}*.scheme$${LITERAL_SQUOTE} -o -name $${LITERAL_SQUOTE}*.desktop$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) $${LITERAL_BACKSLASH}) -o\
        # qds
        $${LITERAL_BACKSLASH}( -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/etc/qds/*$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) -o\
        # pics
        $${LITERAL_BACKSLASH}( $${LITERAL_BACKSLASH}( -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/pics/*$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) -a\
           # config files (media player skins)
           $${LITERAL_BACKSLASH}( -name $${LITERAL_SQUOTE}config$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) $${LITERAL_BACKSLASH}) -o\
        # services (all files)
        $${LITERAL_BACKSLASH}( -path $${LITERAL_SQUOTE}$$QTOPIA_DEPOT_PATH/services/*$${LITERAL_SQUOTE} $${LITERAL_BACKSLASH}) |\
    $$QBS_BIN/nct_lupdate\
        -nowarn\
        -depot\
        $$LITERAL_QUOTE$$QTOPIA_DEPOT_PATH$$LITERAL_QUOTE\
        $$LITERAL_QUOTE$$DQTDIR$$LITERAL_QUOTE\
        $$LITERAL_QUOTE$$ALL_TRANSLATIONS$$LITERAL_QUOTE\
        $$LITERAL_QUOTE$$STRING_LANGUAGE$$LITERAL_QUOTE\
        -
lupdate.depends+=nct_lupdate
QMAKE_EXTRA_TARGETS+=nct_lupdate

VPATH+=.. ../phone
INCLUDEPATH+=.. ../phone

enable_singleexec:qtuitest {
    CONFIG+=qtestlib
}

#test adapted from qtopiacomm.pro
defineTest(setupTranslatables) {
    file=$${1}
    !include($$file) {
        message(Cannot include $${file})
    }
    VARS=HEADERS SOURCES FORMS PRIVATE_HEADERS
    for(v,VARS) {
        eval(TRANSLATABLES*=\$${$$v})
    }
    export(TRANSLATABLES)
    export(VPATH)
}

# Recursively search for .pri files containing translatable files
defineTest(findTranslatables) {
    dir=$$1
    files=$$files($$dir/*)
    for(f,files) {
        # Stop a badly-named file from causing an infinite loop
        containsre($$f,\\\\$):next()
        equals(f,$$QTOPIA_DEPOT_PATH/src/server/main):next()
        file=$$f/$$tail($$f).pri
        exists($$file):setupTranslatables($$file)
        else:findTranslatables($$f)
    }
}
# Only if we're actually processing main.pro!
equals(QTOPIA_ID,server/main):findTranslatables($$QTOPIA_DEPOT_PATH/src/server)

# Depend on all server projects in PROJECTS
for(p,PROJECTS) {
    containsre($$p,^server\/):!equals(p,server/main) {
        depends($$p)
    }
}

# This is documented in src/build/doc/src/deviceprofiles.qdoc
!isEmpty(device) {
    # If server.pri exists, pull it in
    exists($$DEVICE_CONFIG_PATH/server/server.pri) {
        VPATH+=$$DEVICE_CONFIG_PATH/server
        INCLUDEPATH+=$$DEVICE_CONFIG_PATH/server
        include($$DEVICE_CONFIG_PATH/server/server.pri)
    } else {
        h=$$files($$DEVICE_CONFIG_PATH/server/*.h)
        s=$$files($$DEVICE_CONFIG_PATH/server/*.cpp)
        f=$$files($$DEVICE_CONFIG_PATH/server/*.ui)
        !isEmpty(h)|!isEmpty(s)|!isEmpty(f) {
            warning("Pulling in server/*.ui, server/*.h and server/*.cpp is deprecated. You should provide a server.pri file instead.")
            HEADERS+=$$h
            SOURCES+=$$s
            FORMS+=$$f
            INCLUDEPATH+=$$DEVICE_CONFIG_PATH/server
        }
    }
}

# FIXME THIS SHOULD NOT BE HERE!!!
dep(INCLUDEPATH+=$$PWD)
dep(pkg.deps+=$$pkg.name)

enable_singleexec {
    # We need to link to all Qt libs
    QT*=core gui network xml sql svg
    script:QT*=script

    # Final preparation for singleexec build
    # This is hidden from other .pro files because it is very expensive
    equals(QTOPIA_ID,server/main) {
        # The server can't be built as a .a file
        CONFIG-=enable_singleexec
        # However, we still need this define so singleexec can be detected in code
        DEFINES+=SINGLE_EXEC
        # This makes server.pro take an unreasonable amount of time to process.
        # It seems to be p ^ d runs where p = number of projects and d = number of dependencies.
        CONFIG-=link_prl

        cmds=$$fromfile(singleexec_reader.pri,SINGLEEXEC_READER_CMD)
        #message(singleexec_reader tells us:)
        for(c,cmds) {
            contains(QMAKE_BEHAVIORS,keep_quotes) {
                c~=s/^"//
                c~=s/"$//
            }
            #message($$c)
            runlast($$c)
        }
        # This avoids problems due to incorrect ordering of libs.
        # I think it tells the linker to treat all these seperate files as one giant library.
        # Surely this is slow and ineffienent but it does seem to work :)
        cmd=LIBS=-Wl,-whole-archive \$$LIBS -Wl,-no-whole-archive
        runlast($$cmd)
        #message(Done! $$LIBS)
    }

    # qmake isn't putting in these dependencies so do it outselves
    files=$$files($$QPEDIR/lib/*.a)
    files*=$$files($$QTEDIR/lib/*.a)
    qpe.depends=$$files
    qpe.commands=
    QMAKE_EXTRA_TARGETS+=qpe

    ipatchqt.commands=$$COMMAND_HEADER\
        $$fixpath($$QBS_BIN/patchqt) $$fixpath($(INSTALL_ROOT)/bin/qpe) $$QTOPIA_PREFIX
    ipatchqt.CONFIG=no_path
    ipatchqt.depends=install_target
    INSTALLS+=ipatchqt
} else {
    # This makes server.pro take an unreasonable amount of time to process.
    # It seems to be p ^ d runs where p = number of projects and d = number of dependencies.
    CONFIG-=link_prl

    # This avoids problems due to incorrect ordering of libs.
    # I think it tells the linker to treat all these seperate files as one giant library.
    # Surely this is slow and ineffienent but it does seem to work :)
    cmd=LIBS=-Wl,-whole-archive \$$LIBS -Wl,-no-whole-archive
    runlast($$cmd)
}
