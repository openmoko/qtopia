# Implicit dependencies

###
### NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
###
### This code must match the logic found in bin/Qtopia/BlackMagic.pm
###
### NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
###

qt {
    part_of_qtopiadesktop|qtopiadesktop|contains(PROJECT_TYPE,desktop) {
        !containsre($$QTOPIA_ID,^libraries/qt):depends(libraries/qt/*)
    } else {
        !containsre($$QTOPIA_ID,^libraries/qtopiacore):depends(libraries/qtopiacore/*)
    }
}

qtopia {
    enable_qtopiabase:!no_qtopiabase:depends(libraries/qtopiabase)
    !contains(PROJECT_TYPE,core):depends(libraries/qtopia)
}

qtopiadesktop:!contains(PROJECT_TYPE,core) {
    plugin:depends(libraries/qtopiadesktop)
}
