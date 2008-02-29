#####################################################################
# Main projectfile
#####################################################################

CONFIG += ordered
TEMPLATE = subdirs
isEmpty(QT_PROJECTS) {
   #fallback defaults
#  SUBDIRS = qmake
   SUBDIRS += src
   !cross_compile:SUBDIRS += tools
   else:SUBDIRS += tools/qtestlib
   SUBDIRS += demos examples
} else {
   #make sure the order makes sense
   contains(QT_PROJECTS, tools) {
       QT_PROJECTS -= tools
       QT_PROJECTS = tools $$QT_PROJECTS
   }
   contains(QT_PROJECTS, libs) {
       QT_PROJECTS -= libs
       QT_PROJECTS = libs $$QT_PROJECTS
   }
   contains(QT_PROJECTS, qmake) {
       QT_PROJECTS -= qmake
       QT_PROJECTS = qmake $$QT_PROJECTS
   }

   #process the projects
   for(PROJECT, $$list($$lower($$unique(QT_PROJECTS)))) {
       isEqual(PROJECT, tools) {
          !cross_compile:SUBDIRS += tools
          else:SUBDIRS += tools/qtestlib
       } else:isEqual(PROJECT, examples) {
          SUBDIRS += demos examples
       } else:isEqual(PROJECT, libs) {
          SUBDIRS += src
       } else:isEqual(PROJECT, qmake) {
#         SUBDIRS += qmake
       } else {
          message(Unknown PROJECT: $$PROJECT)
       }
   }
}


unix {
  confclean.depends += clean
  confclean.commands += (cd config.tests/unix/stl && $(MAKE) distclean); \
			(cd config.tests/unix/endian && $(MAKE) distclean); \
			(cd config.tests/unix/ipv6 && $(MAKE) distclean); \
			(cd config.tests/unix/largefile && $(MAKE) distclean); \
			(cd config.tests/unix/ptrsize && $(MAKE) distclean); \
			(cd config.tests/x11/notype && $(MAKE) distclean); \
			(cd config.tests/unix/getaddrinfo && $(MAKE) distclean); \
			(cd config.tests/unix/cups && $(MAKE) distclean); \
			(cd config.tests/unix/psql && $(MAKE) distclean); \
			(cd config.tests/unix/mysql && $(MAKE) distclean); \
 	 		(cd config.tests/unix/mysql_r && $(MAKE) distclean); \
			(cd config.tests/unix/nis && $(MAKE) distclean); \
			(cd config.tests/unix/nix && $(MAKE) distclean); \
			(cd config.tests/unix/odbc && $(MAKE) distclean); \
			(cd config.tests/unix/oci && $(MAKE) distclean); \
			(cd config.tests/unix/tds && $(MAKE) distclean); \
			(cd config.tests/unix/db2 && $(MAKE) distclean); \
			(cd config.tests/unix/ibase && $(MAKE) distclean); \
			(cd config.tests/unix/ipv6ifname && $(MAKE) distclean); \
			(cd config.tests/unix/zlib && $(MAKE) distclean); \
			(cd config.tests/unix/libmng && $(MAKE) distclean); \
			(cd config.tests/unix/sqlite2 && $(MAKE) distclean); \
			(cd config.tests/unix/libjpeg && $(MAKE) distclean); \
			(cd config.tests/unix/libpng && $(MAKE) distclean); \
			(cd config.tests/x11/xcursor && $(MAKE) distclean); \
			(cd config.tests/x11/xrender && $(MAKE) distclean); \
			(cd config.tests/x11/xrandr && $(MAKE) distclean); \
			(cd config.tests/x11/xkb && $(MAKE) distclean); \
			(cd config.tests/x11/xinput && $(MAKE) distclean); \
			(cd config.tests/x11/fontconfig && $(MAKE) distclean); \
			(cd config.tests/x11/xinerama && $(MAKE) distclean); \
			(cd config.tests/x11/sm && $(MAKE) distclean); \
			(cd config.tests/x11/xshape && $(MAKE) distclean); \
			(cd config.tests/x11/opengl && $(MAKE) distclean); \
 			(cd qmake && $(MAKE) distclean); \
			$(DEL_FILE) .qmake.cache
  QMAKE_EXTRA_UNIX_TARGETS += confclean
  qmakeclean.commands += (cd qmake && $(MAKE) clean)
  QMAKE_EXTRA_UNIX_TARGETS += qmakeclean
  CLEAN_DEPS += qmakeclean
}
CONFIG -= qt

### installations ####

#docs
htmldocs.files = $$QT_BUILD_TREE/doc/html/*
htmldocs.path = $$[QT_INSTALL_DOCS]/html
INSTALLS += htmldocs

#translations
translations.path=$$[QT_INSTALL_TRANSLATIONS]
translations.files = $$QT_BUILD_TREE/translations/*.qm
INSTALLS += translations

#qmake
qmake.path=$$[QT_INSTALL_BINS]
win32 {
   qmake.files=$$QT_BUILD_TREE/bin/qmake.exe
} else {
   qmake.files=$$QT_BUILD_TREE/bin/qmake
}
INSTALLS += qmake

#mkspecs
mkspecs.path=$$[QT_INSTALL_DATA]/mkspecs
mkspecs.files=$$QT_BUILD_TREE/mkspecs/qconfig.pri $$QT_SOURCE_TREE/mkspecs/*
unix:mkspecs.commands = $(DEL_FILE) $(INSTALL_ROOT)$$mkspecs.path/default; $(SYMLINK) $$basename(QMAKESPEC) $(INSTALL_ROOT)$$mkspecs.path/default
INSTALLS += mkspecs

false:macx { #mac install location
    macdocs.files = $$htmldocs.files
    macdocs.path = /Developer/Documentation/Qt
    INSTALLS += macdocs
}

!win32:contains(QT_CONFIG, qtusagereporter) {
    usagereporter.path=$$[QT_INSTALL_BINS]
    usagereporter.files=$$QT_BUILD_TREE/bin/qtusagereporter
    INSTALLS += usagereporter
}