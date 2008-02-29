TEMPLATE=subdirs

include($${QTOPIA_DEPOT_PATH}/src/projects.pri)

# Not ordered
SUBDIRS+=$${APP_PROJECTS} $${PLUGIN_PROJECTS} $${SERVER}
!isEmpty(THEME_PROJECTS):SUBDIRS+=../etc/themes
!isEmpty(EXTRA_SUBDIRS):SUBDIRS+=$${EXTRA_SUBDIRS}

