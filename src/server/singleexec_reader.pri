# This file attempts to speed up requests for multiple values from individual .pro files.
# There are various problems that must be overcome (such as saving/restoring variables and
# forcing feature files to be processed more than once). All this work is in a separate
# file to avoid "contaminating" the server's project with extra variables.


# Don't process the lupdate rule for the Qt directory (it takes too long)
CONFIG+=disable_qt_lupdate
# This is the variable that server.pro is going to read
SINGLEEXEC_READER_CMD=

# We need to check all the project roots because device-specific code may exist
# outside the Qtopia project root.
for(pr,PROJECT_ROOTS) {
    # Save some variables
    TMP2_PROCESSED_PRI=$$PROCESSED_PRI
    TMP2_CONFIG=$$CONFIG

    # Clear some variables
    PROJECTS=
    CONFIG-=qtopia part_of_qtopia
    DEP_PROJECTS=

    # Read in the tree configuration and project list
    include($$pr/tree_config.pri)
    include($$pr/projects.pri)

    for(p,PROJECTS) {
        # Don't cause an infinite loop!
        equals(pr,$$QTOPIA_PROJECT_ROOT):equals(p,server):next()

        # This is the .pro file we expect to find
        file=$$fixpath($$pr/$$p/$$tail($$p).pro)
        # We can't do anything if we can't read the .pro file
        !exists($$file) {
            warning(Missing $$file)
            next()
        }

        # Save some variables
        TMP_PROCESSED_PRI=$$PROCESSED_PRI
        TMP_CONFIG=$$CONFIG
        TMP_PROJECT_TYPE=$$PROJECT_TYPE

        # Clear some variables
        QTOPIA_DEPENDS=
        QTOPIA_DEPENDS_NO_WARN=
        PROCESSED_DEPS=
        LIBS=
        plugin_type=

        # Projects use this to identify themselves. It's normally set by config.prf but we have to do it
        # manually because config.prf has already been read.
        QTOPIA_ID=$$p

        include($$file)

        SINGLEEXEC_READER_ADD=0
        # Only Qtopia projects are considered
        part_of_qtopia|qtopia {
            # Only apps, plugins and libs are considered
            contains(PROJECT_TYPE,app)|contains(PROJECT_TYPE,plugin)|contains(PROJECT_TYPE,lib) {
                enable_singleexec:SINGLEEXEC_READER_ADD=1

                # The code that sets these doesn't seem to run
                contains(PROJECT_TYPE,app):CONFIG+=app
                contains(PROJECT_TYPE,plugin):CONFIG+=plugin
                contains(PROJECT_TYPE,lib):CONFIG+=lib

                app:!singleexec_main:!qtopia_main:CONFIG+=no_quicklaunch no_singleexec
            }
        }

        no_singleexec:SINGLEEXEC_READER_ADD=0

        equals(SINGLEEXEC_READER_ADD,1) {
            # This is needed for plugin_type
            qtopia:include($$QTOPIA_DEPOT_PATH/src/build/qtopia.prf)

            DEP_PROJECTS*=$$QTOPIA_DEPENDS

            # Stop the depends file from being read again
            CONFIG-=depends
        }

        # Check again because a project we depend on might have set this
        no_singleexec:SINGLEEXEC_READER_ADD=0

        equals(SINGLEEXEC_READER_ADD,1) {
            # We need some plugin info
            plugin:include($$QTOPIA_DEPOT_PATH/src/build/plugin.prf)
            # Get the TARGET value
            include($$QTOPIA_DEPOT_PATH/src/build/singleexec.prf)
        }

        # Restore the saved variables
        PROCESSED_PRI=$$TMP_PROCESSED_PRI
        CONFIG=$$TMP_CONFIG
        PROJECT_TYPE=$$TMP_PROJECT_TYPE
        
        # Enable these lines to see which projects are accepted and which are rejected
        #equals(SINGLEEXEC_READER_ADD,1):message(YES: $$pr/$$p (-l$$TARGET $$LIBS))
        #equals(SINGLEEXEC_READER_ADD,0):message(NO : $$pr/$$p)

        # If we're going to use this project, add the LIBS line (including the project's target)
        # to the list of things server.pro should do
        equals(SINGLEEXEC_READER_ADD,1):SINGLEEXEC_READER_CMD+="LIBS*=-l$$TARGET $$LIBS"

        # This doesn't work. Something like it could work but you'd probably have to
        # know what the dependancy heirachy is to get it right.
        #DEP_PROJECTS-=$$PROJECTS
    }

    !isEmpty(DEP_PROJECTS) {
        # Load up the dependency libs
        LIBS=
        QTOPIA_DEPENDS=$$DEP_PROJECTS
        include($$QTOPIA_DEPOT_PATH/src/build/depends.prf)
        SINGLEEXEC_READER_CMD+="LIBS*=$$LIBS"
    }

    # Restore the saved variables
    PROCESSED_PRI=$$TMP2_PROCESSED_PRI
    CONFIG=$$TMP2_CONFIG
}

