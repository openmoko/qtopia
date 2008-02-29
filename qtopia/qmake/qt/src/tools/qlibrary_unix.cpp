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

#include "qplatformdefs.h"
#include "private/qlibrary_p.h"

#ifndef QT_NO_LIBRARY

#if defined(QT_AOUT_UNDERSCORE)
#include <string.h>
#endif

/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol

  It's not too hard to guess what the functions do.
*/

#if defined(Q_OS_MAC)

bool QLibraryPrivate::loadLibrary()
{
    return FALSE;
}

bool QLibraryPrivate::freeLibrary()
{
    return FALSE;
}

void* QLibraryPrivate::resolveSymbol( const char* )
{
    return 0;
}

#elif defined(QT_HPUX_LD) // for HP-UX < 11.x and 32 bit

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    QString filename = library->library();

    pHnd = (void*)shl_load( filename.latin1(), BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0 );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !pHnd )
	qWarning( "%s: failed to load library!", filename.latin1() );
#endif
    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;

    if ( shl_unload( (shl_t)pHnd ) ) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	QString filename = library->library();
	qWarning( "%s: Failed to unload library!", filename.latin1() );
#endif
	return FALSE;
    }
    pHnd = 0;
    return TRUE;
}

void* QLibraryPrivate::resolveSymbol( const char* symbol )
{
    if ( !pHnd )
	return 0;

    void* address = 0;
    if ( shl_findsym( (shl_t*)&pHnd, symbol, TYPE_UNDEFINED, &address ) < 0 ) {
#if defined(QT_DEBUG_COMPONENT)
	QString filename = library->library();
	qWarning( "%s: couldn't resolve symbol \"%s\"", filename.latin1(), symbol );
#endif
    }
    return address;
}

#else // POSIX
#include <dlfcn.h>

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    QString filename = library->library();

    pHnd = dlopen( filename.latin1(), RTLD_LAZY );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !pHnd )
	qWarning( "%s", dlerror() );
#endif
    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;

    if ( dlclose( pHnd ) ) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	qWarning( "%s", dlerror() );
#endif
	return FALSE;
    }

    pHnd = 0;
    return TRUE;
}

void* QLibraryPrivate::resolveSymbol( const char* symbol )
{
    if ( !pHnd )
	return 0;

#if defined(QT_AOUT_UNDERSCORE)
    // older a.out systems add an underscore in front of symbols
    char* undrscr_symbol = new char[strlen(symbol)+2];
    undrscr_symbol[0] = '_';
    strcpy(undrscr_symbol+1, symbol);
    void* address = dlsym( pHnd, undrscr_symbol );
    delete [] undrscr_symbol;
#else
    void* address = dlsym( pHnd, symbol );
#endif
#if defined(QT_DEBUG_COMPONENT)
    const char* error = dlerror();
    if ( error )
	qWarning( "%s", error );
#endif
    return address;
}

#endif // POSIX

#endif
