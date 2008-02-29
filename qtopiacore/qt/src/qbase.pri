isEmpty(TARGET):error(You must set TARGET before include()'ing $${_FILE_})
INCLUDEPATH *= $$QMAKE_INCDIR_QT/$$TARGET #just for today to have some compat
isEmpty(QT_ARCH):!isEmpty(ARCH):QT_ARCH=$$ARCH #another compat that will rot for change #215700
TEMPLATE	= lib
isEmpty(QT_MAJOR_VERSION) {
   VERSION=4.2.3
} else {
   VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
}
mac:!contains(QMAKE_EXT_C, .mm):QMAKE_EXT_C += .mm

#load up the headers info
CONFIG += qt_install_headers
HEADERS_PRI = $$QT_BUILD_TREE/include/$$TARGET/headers.pri
include($$HEADERS_PRI)|clear(HEADERS_PRI)

#version overriding
win32 {
    #because libnetwork.pro could be qmake'd (qmade?) before libqcore.pro we
    #need to override the version of libq* in all other libq*'s just to be
    #sure the same version is used
	VERSIONS_LIST = $$split(VERSION, ".")
    QT_LIBS_OVERRIDE = $$member(VERSIONS_LIST, 0)
	for(lib, $$list(qtcore qtgui qtnetwork qtxml qtopengl qtsql qt3support)) {
        eval(QMAKE_$${upper($$lib)}_VERSION_OVERRIDE = $$QT_LIBS_OVERRIDE)
	eval(QMAKE_$${upper($$lib)}D_VERSION_OVERRIDE = $$QT_LIBS_OVERRIDE)
    }
}

#other
DESTDIR		 = $$QMAKE_LIBDIR_QT
win32:DLLDESTDIR = $$[QT_INSTALL_PREFIX]/bin

CONFIG		+= qt warn_on depend_includepath
CONFIG          += qmake_cache target_qt 
CONFIG          -= fix_output_dirs
win32|mac:!macx-xcode:CONFIG += debug_and_release

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
contains(QT_CONFIG, largefile):CONFIG += largefile

#mac frameworks
mac:!static:contains(QT_CONFIG, qt_framework) {
   #QMAKE_FRAMEWORK_VERSION = 4.0
   QMAKE_FRAMEWORK_BUNDLE_NAME = $$TARGET
   CONFIG += lib_bundle qt_no_framework_direct_includes qt_framework
   CONFIG(debug, debug|release) {
      !build_pass:CONFIG += build_all
   } else { #release
      !debug_and_release|build_pass {
	  CONFIG -= qt_install_headers #no need to install these as well
	  FRAMEWORK_HEADERS.version = Versions
	  FRAMEWORK_HEADERS.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
      	  FRAMEWORK_HEADERS.path = Headers
      	  equals(TARGET, QtCore) {
              #headers generated by configure
              !contains(FRAMEWORK_HEADERS.files, .*/qconfig.h) {
	          FRAMEWORK_HEADERS.files *= $$QT_BUILD_TREE/src/corelib/global/qconfig.h 
	      }
          }
      }
      QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
   }
}

mac {
   CONFIG += explicitlib
   QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.2 #enables weak linking for 10.2 (exported)
   macx-g++ {
       QMAKE_CFLAGS += -fconstant-cfstrings
       QMAKE_CXXFLAGS += -fconstant-cfstrings
   }
}

win32:!shared:CONFIG += static

win32-borland {
    mng:QMAKE_CFLAGS_WARN_ON	+= -w-par
    mng:QMAKE_CXXFLAGS_WARN_ON	+= -w-par
    # Keep the size of the .tds file for the Qt library smaller than
    # 34 Mbytes to avoid linking problems
    QMAKE_CFLAGS_DEBUG += -vi -y-
    QMAKE_CXXFLAGS_DEBUG += -vi -y-
}

win32 {
    CONFIG += zlib
    INCLUDEPATH += tmp
    !static: DEFINES+=QT_MAKEDLL
}
win32-borland:INCLUDEPATH += kernel

aix-g++* {
     QMAKE_CFLAGS   += -mminimal-toc
     QMAKE_CXXFLAGS += -mminimal-toc
}

embedded {
     EMBEDDED_H = $$EMBEDDED_CPP
}

DEPENDPATH += ;$$NETWORK_H;$$KERNEL_H;$$WIDGETS_H;$$SQL_H;$$TABLE_H;$$DIALOGS_H;
DEPENDPATH += $$ICONVIEW_H;$$OPENGL_H;$$THREAD_H;$$TOOLS_H;$$CODECS_H;
DEPENDPATH += $$WORKSPACE_H;$$XML_H;$$STYLES_H;$$COMPAT_H
embedded:DEPENDPATH += ;$$EMBEDDED_H

!static:PRL_EXPORT_DEFINES += QT_SHARED

#install directives
include(qt_install.pri)

unix {
   CONFIG     += create_libtool create_pc explicitlib
   QMAKE_PKGCONFIG_LIBDIR = $$[QT_INSTALL_LIBS]
   QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_HEADERS]/$$TARGET
   QMAKE_PKGCONFIG_CFLAGS = -I$$[QT_INSTALL_HEADERS]
}

contains(QT_PRODUCT, OpenSource.*):DEFINES *= QT_OPENSOURCE
DEFINES += QT_NO_CAST_TO_ASCII QT_ASCII_CAST_WARNINGS
contains(QT_CONFIG, qt3support):DEFINES *= QT3_SUPPORT
DEFINES *= QT_MOC_COMPAT #we don't need warnings from calling moc code in our generated code

!debug_and_release|build_pass {
   CONFIG(debug, debug|release) {
      mac:TARGET = $$member(TARGET, 0)_debug
      win32:TARGET = $$member(TARGET, 0)d
   }
}

moc_dir.name = moc_location
moc_dir.variable = QMAKE_MOC

uic_dir.name = uic_location
uic_dir.variable = QMAKE_UIC

QMAKE_PKGCONFIG_VARIABLES += moc_dir uic_dir

include(qt_targets.pri)

