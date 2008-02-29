# This file performs marshalling of the various other .pri files
# It also sets up LIBRARY_PROJECTS properly (core libs go first)

!contains( processed_pri, projects.pri ) {
    processed_pri += projects.pri

    include($${QTOPIA_DEPOT_PATH}/src/general.pri)
    include($${QTOPIA_DEPOT_PATH}/src/commercial.pri)
    include($${QTOPIA_DEPOT_PATH}/src/custom.pri)
    include($$(QPEDIR)/src/local.pri)

    LIBRARY_PROJECTS=$$CORE_LIBRARY_PROJECTS $$LIBRARY_PROJECTS
}

