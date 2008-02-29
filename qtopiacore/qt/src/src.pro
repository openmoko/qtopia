TEMPLATE = subdirs

# this order is important
win32:SUBDIRS += src_winmain
SUBDIRS += src_tools_moc src_tools_rcc src_tools_uic src_corelib src_xml src_gui src_sql src_network src_svg
!embedded:contains(QT_CONFIG, opengl): SUBDIRS += src_opengl
contains(QT_CONFIG, qt3support): SUBDIRS += src_qt3support
!cross_compile {
    contains(QT_CONFIG, qt3support): SUBDIRS += src_tools_uic3
}
win32:!contains(QT_EDITION, OpenSource|Console):SUBDIRS += src_activeqt src_tools_idc
SUBDIRS += src_plugins

src_winmain.subdir = winmain
src_tools_moc.subdir = tools/moc
src_tools_rcc.subdir = tools/rcc
src_tools_uic.subdir = tools/uic
src_corelib.subdir = corelib
src_xml.subdir = xml
src_gui.subdir = gui
src_sql.subdir = sql
src_network.subdir = network
src_svg.subdir = svg
src_opengl.subdir = opengl
src_qt3support.subdir = qt3support
src_tools_uic3.subdir = tools/uic3
src_activeqt.subdir = activeqt
src_tools_idc.subdir = tools/idc
src_plugins.subdir = plugins

#CONFIG += ordered
!ordered {
   src_corelib.depends = src_tools_moc src_tools_rcc
   src_gui.depends = src_corelib src_tools_uic
   src_xml.depends = src_corelib
   src_svg.depends = src_xml src_gui
   src_network.depends = src_corelib
   src_opengl.depends = src_gui
   src_sql.depends = src_corelib
   src_qt3support.depends = src_gui src_xml src_network src_sql
   src_tools_uic3.depends = src_qt3support src_xml
   src_tools_idc.depends = src_corelib
   src_tools_activeqt.depends = src_tools_idc src_gui
   src_plugins.depends = src_gui src_sql src_svg
   contains(QT_CONFIG, qt3support): src_plugins.depends += src_qt3support
}


# This gives us a top level debug/release
EXTRA_DEBUG_TARGETS =
EXTRA_RELEASE_TARGETS =
for(subname, SUBDIRS) {
   subdir = $$subname
   !isEmpty($${subname}.subdir):subdir = $$eval($${subname}.subdir)
   subpro = $$subdir/$${basename(subdir)}.pro
   !exists($$subpro):next()
   isEqual($$list($$fromfile($$subpro, TEMPLATE)), lib) {
       #debug
       eval(debug-$${subdir}.depends = $${subdir}/$(MAKEFILE) $$EXTRA_DEBUG_TARGETS)
       eval(debug-$${subdir}.commands = (cd $$subdir && $(MAKE) -f $(MAKEFILE) debug))
       EXTRA_DEBUG_TARGETS += debug-$${subdir}
       QMAKE_EXTRA_TARGETS += debug-$${subdir}
       #release
       eval(release-$${subdir}.depends = $${subdir}/$(MAKEFILE) $$EXTRA_RELEASE_TARGETS)
       eval(release-$${subdir}.commands = (cd $$subdir && $(MAKE) -f $(MAKEFILE) release))
       EXTRA_RELEASE_TARGETS += release-$${subdir}
       QMAKE_EXTRA_TARGETS += release-$${subdir}
    } else { #do not have a real debug target/release
       #debug
       eval(debug-$${subdir}.depends = $${subdir}/$(MAKEFILE) $$EXTRA_DEBUG_TARGETS)
       eval(debug-$${subdir}.commands = (cd $$subdir && $(MAKE) -f $(MAKEFILE) first))
       EXTRA_DEBUG_TARGETS += debug-$${subdir}
       QMAKE_EXTRA_TARGETS += debug-$${subdir}
       #release
       eval(release-$${subdir}.depends = $${subdir}/$(MAKEFILE) $$EXTRA_RELEASE_TARGETS)
       eval(release-$${subdir}.commands = (cd $$subdir && $(MAKE) -f $(MAKEFILE) first))
       EXTRA_RELEASE_TARGETS += release-$${subdir}
       QMAKE_EXTRA_TARGETS += release-$${subdir}
   }
}
debug.depends = $$EXTRA_DEBUG_TARGETS
release.depends = $$EXTRA_RELEASE_TARGETS
QMAKE_EXTRA_TARGETS += debug release
