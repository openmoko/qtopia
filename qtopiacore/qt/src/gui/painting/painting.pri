# Qt gui library, paint module

HEADERS += \
	painting/qbezier_p.h \
	painting/qbrush.h \
	painting/qcolor.h \
	painting/qcolor_p.h \
	painting/qcolormap.h \
	painting/qdrawutil.h \
	painting/qpaintdevice.h \
	painting/qpaintengine.h \
	painting/qpainter.h \
	painting/qpainter_p.h \
	painting/qpainterpath.h \
	painting/qpainterpath_p.h \
	painting/qpdf_p.h \
	painting/qpen.h \
	painting/qpolygon.h \
	painting/qpolygonclipper_p.h \
	painting/qprintengine_pdf_p.h \
	painting/qprintengine_ps_p.h \
	painting/qprinter.h \
	painting/qprinter_p.h \
	painting/qprintengine.h \
	painting/qregion.h \
	painting/qstroker_p.h \
        painting/qstylepainter.h \
	painting/qmatrix.h \
	painting/qwmatrix.h


SOURCES += \
	painting/qbezier.cpp \
	painting/qbrush.cpp \
	painting/qcolor.cpp \
	painting/qcolor_p.cpp \
	painting/qdrawutil.cpp \
	painting/qpaintengine.cpp \
	painting/qpainter.cpp \
	painting/qpainterpath.cpp \
	painting/qpdf.cpp \
	painting/qpen.cpp \
	painting/qpolygon.cpp \
	painting/qprintengine_pdf.cpp \
	painting/qprintengine_ps.cpp \
	painting/qprinter.cpp \
	painting/qstroker.cpp \
        painting/qstylepainter.cpp \
	painting/qregion.cpp \
	painting/qmatrix.cpp

	DEFINES += QT_RASTER_IMAGEENGINE
        win32:DEFINES += QT_RASTER_PAINTENGINE
        embedded:DEFINES += QT_RASTER_PAINTENGINE
	SOURCES += 					\
	 	painting/qpaintengine_raster.cpp	\
		painting/qdrawhelper.cpp 		\
		painting/qblackraster.c			\
		painting/qgrayraster.c

	HEADERS += 					\
		painting/qpaintengine_raster_p.h 	\
		painting/qrasterdefs_p.h		\
		painting/qgrayraster_p.h		\
		painting/qblackraster_p.h

win32 {
	HEADERS += \
		painting/qprintengine_win_p.h

 	SOURCES += \
		painting/qcolormap_win.cpp \
		painting/qpaintdevice_win.cpp \
		painting/qprintengine_win.cpp \
		painting/qregion_win.cpp

	!win32-borland:LIBS += -lmsimg32
}

wince-* {
	SOURCES -= painting/qregion_win.cpp
	SOURCES += painting/qregion_wce.cpp
}


unix:x11 {
	HEADERS += \
		painting/qpaintengine_x11_p.h

	SOURCES += \
		painting/qcolormap_x11.cpp \
		painting/qpaintdevice_x11.cpp \
		painting/qpaintengine_x11.cpp
}

!embedded:!x11:mac {
	HEADERS += \
		painting/qpaintengine_mac_p.h \
		painting/qprintengine_mac_p.h

	SOURCES += \
		painting/qcolormap_mac.cpp \
		painting/qpaintdevice_mac.cpp \
		painting/qpaintengine_mac.cpp \
		painting/qprintengine_mac.cpp
}

unix:SOURCES += painting/qregion_unix.cpp

win32|x11|embedded {
        SOURCES += painting/qbackingstore.cpp
}

embedded {
        contains(QT_CONFIG,qtopia) {
		DEFINES += QTOPIA_PRINTENGINE
		HEADERS += painting/qprintengine_qws_p.h
		SOURCES += painting/qprintengine_qws.cpp
        }
                
	SOURCES += \
		painting/qcolormap_qws.cpp \
		painting/qpaintdevice_qws.cpp
} 

x11|embedded {
        contains(QT_CONFIG,qtopia) {
            DEFINES += QT_NO_CUPS QT_NO_LPR
        } else {
            SOURCES += painting\qcups.cpp
            HEADERS += painting\qcups_p.h
        }
} else {
	DEFINES += QT_NO_CUPS QT_NO_LPR
}

mac {

} else:sse|win32-g++ {
    sse_compiler.commands = $$QMAKE_CXX -c -msse $(CXXFLAGS) $(INCPATH) ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
    sse_compiler.dependency_type = TYPE_C
    sse_compiler.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_OBJ)}
    sse_compiler.input = SSE_SOURCES
    sse_compiler.variable_out = OBJECTS
    sse_compiler.name = compiling[sse] ${QMAKE_FILE_IN}
    silent:sse_compiler.commands = @echo compiling[sse] ${QMAKE_FILE_IN} && $$sse_compiler.commands
    QMAKE_EXTRA_COMPILERS += sse_compiler
    DEFINES += QT_HAVE_SSE

    SSE_SOURCES += painting/qdrawhelper_x86.cpp
} else:iwmmxt {
    SOURCES += painting/qdrawhelper_x86.cpp
    DEFINES += QT_HAVE_IWMMXT
    DEFINES += QT_HAVE_SSE
} else:win32:!win32-msvc {
    SOURCES += painting/qdrawhelper_x86.cpp
    DEFINES += QT_HAVE_SSE
}

win32|x11|embedded {
	HEADERS += painting/qwindowsurface_p.h \
		   painting/qwindowsurface_raster_p.h
	SOURCES += painting/qwindowsurface_raster.cpp

	x11 {
		HEADERS += painting/qwindowsurface_x11_p.h
		SOURCES += painting/qwindowsurface_x11.cpp
	}

	embedded {
		HEADERS += painting/qwindowsurface_qws_p.h 
		SOURCES += painting/qwindowsurface_qws.cpp
	}
}

