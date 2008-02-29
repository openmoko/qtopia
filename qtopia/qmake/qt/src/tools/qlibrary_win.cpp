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

#include <qmap.h>
#include <private/qlibrary_p.h>

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

#ifndef QT_H
#include "qfile.h"
#endif // QT_H

#ifndef QT_NO_LIBRARY

struct LibInstance {
    LibInstance() { instance = 0; refCount = 0; }
    HINSTANCE instance;
    int refCount;
};

static QMap<QString, LibInstance*> *map = 0;
/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol

  It's not too hard to guess what the functions do.
*/

#include "qt_windows.h"

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

#ifdef QT_THREAD_SUPPORT
    // protect map creation/access
    QMutexLocker locker( qt_global_mutexpool ?
			 qt_global_mutexpool->get( &map ) : 0 );
#endif // QT_THREAD_SUPPORT

    if ( !map )
	map = new QMap<QString, LibInstance*>;

    QString filename = library->library();
    if ( map->find(filename) != map->end() ) {
	LibInstance *lib = (*map)[filename];
	lib->refCount++;
	pHnd = lib->instance;
    }
    else {
	QT_WA( {
	    pHnd = LoadLibraryW( (TCHAR*)filename.ucs2() );
	} , {
	    pHnd = LoadLibraryA(QFile::encodeName( filename ).data());
	} );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	if ( !pHnd )
	    qSystemWarning( QString("Failed to load library %1!").arg( filename ) );
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

#ifdef QT_THREAD_SUPPORT
    // protect map access
    QMutexLocker locker( qt_global_mutexpool ?
			 qt_global_mutexpool->get( &map ) : 0 );
#endif // QT_THREAD_SUPPORT

    bool ok = FALSE;
    QMap<QString, LibInstance*>::iterator it;
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
		delete lib;
	    } else
		ok = TRUE;
	    break;
	}
    }
    if ( ok )
	pHnd = 0;
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    else
	qSystemWarning( "Failed to unload library!" );
#endif
    return ok;
}

void* QLibraryPrivate::resolveSymbol( const char* f )
{
    if ( !pHnd )
	return 0;

#ifdef Q_OS_TEMP
    void* address = (void*)GetProcAddress( pHnd, (const wchar_t*)QString(f).ucs2() );
#else
    void* address = (void*)GetProcAddress( pHnd, f );
#endif
#if defined(QT_DEBUG_COMPONENT)
    if ( !address )
	qSystemWarning( QString("Couldn't resolve symbol \"%1\"").arg( f ) );
#endif

    return address;
}

#endif //QT_NO_LIBRARY
