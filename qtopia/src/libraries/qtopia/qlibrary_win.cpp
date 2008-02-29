/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/qpeglobal.h>
#include <qmap.h>
#include "qlibrary_p.h"
#if QT_VERSION < 0x030000
#include <qtopia/qpeapplication.h>  // needed for winVersion()
#endif
#ifndef QTOPIA_INTERNAL_FILEOPERATIONS
#define QTOPIA_INTERNAL_FILEOPERATIONS
#endif
#include <windows.h>
#include <winbase.h>

#ifndef QT_H
#include <qfile.h>
#endif // QT_H
struct LibInstance {
    LibInstance() { instance = 0; refCount = 0; }
    HINSTANCE instance;
    int refCount;
};

static QMap<QString, LibInstance*> *map = 0;

#ifndef QT_NO_COMPONENT

/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol

  It's not too hard to guess what the functions do.
*/

bool QLibraryPrivate::loadLibrary()
{
   if ( pHnd )
	return TRUE;

    if ( !map )
	map = new QMap<QString, LibInstance*>;

    QString filename = library->library();
    if ( map->find(filename) != map->end() ) {
	LibInstance *lib = (*map)[filename];
	lib->refCount++;
	pHnd = lib->instance;
    }
    else {
#if defined(UNICODE)
	if ( qt_winunicode )
	    pHnd = LoadLibraryW( (TCHAR*)qt_winTchar( filename, TRUE) );
	else
#endif
	    pHnd = LoadLibraryA(QFile::encodeName( filename ).data());
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	if ( !pHnd )
	    qDebug( QString("Failed to load library %1!").arg( filename ) );
#endif
	if ( pHnd ) {
	    LibInstance *lib = new LibInstance;
	    lib->instance = pHnd;
	    lib->refCount++;
	    map->insert( filename, lib );
	}
    }
    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;
    bool ok = FALSE;
    QMap<QString, LibInstance*>::Iterator it;
    for ( it = map->begin(); it != map->end(); ++it ) {
	LibInstance *lib = *it;
	if ( lib->instance == pHnd ) {
	    lib->refCount--;
	    if ( lib->refCount == 0 ) {
		ok = FreeLibrary( pHnd );
		if ( ok ) {
		    map->remove( it );
		    if ( map->count() == 0 ) {
			delete map;
			map = 0;
		    }
		}
	    } else
		ok = TRUE;
	    break;
	}
    }
    if ( ok )
	pHnd = 0;
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    else
	qDebug( "Failed to unload library!" );
#endif
    return ok;
}

void* QLibraryPrivate::resolveSymbol( const char* f )
{
    if ( !pHnd )
	return 0;

#ifdef Q_OS_TEMP
    void* address = GetProcAddress( pHnd, (TCHAR*)qt_winTchar( f, TRUE) );
#else
    void* address = GetProcAddress( pHnd, f );
#endif
#if defined(QT_DEBUG_COMPONENT)
    if ( !address )
	qDebug( QString("Couldn't resolve symbol \"%1\"").arg( f ) );
#endif

    return address;
}


#endif // QT_NO_COMPONENT
