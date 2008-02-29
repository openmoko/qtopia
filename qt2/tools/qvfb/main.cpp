/****************************************************************************
**
** Qt/Embedded virtual framebuffer
**
** Created : 20000605
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qapplication.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <qregexp.h>

#include "qvfb.h"

void fn_quit_qvfb(int)
{
    // pretend that we have quit normally
    qApp->quit();
}


void usage( const char *app )
{
    printf( "Usage: %s [-width width] [-height height] [-depth depth] "
	    "[-nocursor] [-qwsdisplay :id] [-skin skindirectory]\n"
	    "Supported depths: 1, 4, 8, 18, 32\n", app );
}

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    bool useMap = false;
    int width = 0;
    int height = 0;
    int depth = 32;
    int rotation = 0;
    bool cursor = TRUE;
    double zoom = 1.0;
    QString displaySpec( ":0" );
    QString skin;

    for ( int i = 1; i < argc; i++ ){
	QString arg = argv[i];
	if ( arg == "-width" ) {
	    width = atoi( argv[++i] );
	} else if ( arg == "-height" ) {
	    height = atoi( argv[++i] );
	} else if ( arg == "-skin" ) {
	    skin = argv[++i];
	} else if ( arg == "-depth" ) {
	    depth = atoi( argv[++i] );
	} else if ( arg == "-nocursor" ) {
	    cursor = FALSE;
	} else if ( arg == "-zoom" ) {
	    zoom = atof( argv[++i] );
	} else if ( arg == "-qwsdisplay" ) {
	    displaySpec = argv[++i];
	} else if ( arg == "-mmap" ) {
            useMap = true;
	} else {
	    printf( "Unknown parameter %s\n", arg.latin1() );
	    usage( argv[0] );
	    exit(1);
	}
    }

    int displayId = 0;
    QRegExp r( ":[0-9]+" );
    int len;
    int m = r.match( displaySpec, 0, &len );
    if ( m >= 0 ) {
	displayId = displaySpec.mid( m+1, len-1 ).toInt();
    }
    QRegExp rotRegExp( "Rot[0-9]+" );
    m = rotRegExp.match( displaySpec, 0, &len );
    if ( m >= 0 ) {
	rotation = displaySpec.mid( m+3, len-3 ).toInt();
    }

    qDebug( "Using display %d", displayId );
    signal(SIGINT, fn_quit_qvfb);
    signal(SIGTERM, fn_quit_qvfb);

    QVFb mw( displayId, width, height, depth, rotation, skin, useMap );
    mw.setZoom(zoom);
    app.setMainWidget( &mw );
    mw.enableCursor(cursor);
    mw.show();

    return app.exec();
}


