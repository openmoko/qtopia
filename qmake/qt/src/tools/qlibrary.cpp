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
#include <private/qlibrary_p.h>
#include <qfile.h>

#ifndef QT_NO_LIBRARY

// uncomment this to get error messages
//#define QT_DEBUG_COMPONENT 1
// uncomment this to get error and success messages
//#define QT_DEBUG_COMPONENT 2

#ifndef QT_DEBUG_COMPONENT
# if defined(QT_DEBUG)
#  define QT_DEBUG_COMPONENT 1
# endif
#endif

#if defined(Q_WS_WIN) && !defined(QT_MAKEDLL)
#define QT_NO_LIBRARY_UNLOAD
#endif

QLibraryPrivate::QLibraryPrivate( QLibrary *lib )
    : pHnd( 0 ), library( lib )
{
}


/*!
    \class QLibrary qlibrary.h
    \reentrant
    \brief The QLibrary class provides a wrapper for handling shared libraries.

    \mainclass
    \ingroup plugins

    An instance of a QLibrary object can handle a single shared
    library and provide access to the functionality in the library in
    a platform independent way. If the library is a component server,
    QLibrary provides access to the exported component and can
    directly query this component for interfaces.

    QLibrary ensures that the shared library is loaded and stays in
    memory whilst it is in use. QLibrary can also unload the library
    on destruction and release unused resources.

    A typical use of QLibrary is to resolve an exported symbol in a
    shared object, and to call the function that this symbol
    represents. This is called "explicit linking" in contrast to
    "implicit linking", which is done by the link step in the build
    process when linking an executable against a library.

    The following code snippet loads a library, resolves the symbol
    "mysymbol", and calls the function if everything succeeded. If
    something went wrong, e.g. the library file does not exist or the
    symbol is not defined, the function pointer will be 0 and won't be
    called. When the QLibrary object is destroyed the library will be
    unloaded, making all references to memory allocated in the library
    invalid.

    \code
    typedef void (*MyPrototype)();
    MyPrototype myFunction;

    QLibrary myLib( "mylib" );
    myFunction = (MyProtoype) myLib.resolve( "mysymbol" );
    if ( myFunction ) {
	myFunction();
    }
    \endcode
*/

/*!
    Creates a QLibrary object for the shared library \a filename. The
    library will be unloaded in the destructor.

    Note that \a filename does not need to include the (platform specific)
    file extension, so calling
    \code
    QLibrary lib( "mylib" );
    \endcode
    is equivalent to calling
    \code
    QLibrary lib( "mylib.dll" );
    \endcode
    on Windows, and
    \code
    QLibrary lib( "libmylib.so" );
    \endcode
    on Unix. Specifying the extension is not recommended, since
    doing so introduces a platform dependency.

    If \a filename does not include a path, the library loader will
    look for the file in the platform specific search paths.

    \sa load() unload(), setAutoUnload()
*/
QLibrary::QLibrary( const QString& filename )
    : libfile( filename ), aunload( TRUE )
{
    libfile.replace( '\\', '/' );
    d = new QLibraryPrivate( this );
}

/*!
    Deletes the QLibrary object.

    The library will be unloaded if autoUnload() is TRUE (the
    default), otherwise it stays in memory until the application
    exits.

    \sa unload(), setAutoUnload()
*/
QLibrary::~QLibrary()
{
    if ( autoUnload() )
	unload();

    delete d;
}

/*!
    Returns the address of the exported symbol \a symb. The library is
    loaded if necessary. The function returns 0 if the symbol could
    not be resolved or the library could not be loaded.

    \code
    typedef int (*avgProc)( int, int );

    avgProc avg = (avgProc) library->resolve( "avg" );
    if ( avg )
	return avg( 5, 8 );
    else
	return -1;
    \endcode

*/
void *QLibrary::resolve( const char* symb )
{
    if ( !d->pHnd )
	load();
    if ( !d->pHnd )
	return 0;

    void *address = d->resolveSymbol( symb );

    return address;
}

/*!
    \overload

    Loads the library \a filename and returns the address of the
    exported symbol \a symb. Note that like the constructor, \a
    filename does not need to include the (platform specific) file
    extension. The library remains loaded until the process exits.

    The function returns 0 if the symbol could not be resolved or the
    library could not be loaded.

    This function is useful only if you want to resolve a single
    symbol, e.g. a function pointer from a specific library once:

    \code
    typedef void (*FunctionType)();
    static FunctionType *ptrFunction = 0;
    static bool triedResolve = FALSE;
    if ( !ptrFunction && !triedResolve )
	ptrFunction = QLibrary::resolve( "mylib", "mysymb" );

    if ( ptrFunction )
	ptrFunction();
    else
	...
    \endcode

    If you want to resolve multiple symbols, use a QLibrary object and
    call the non-static version of resolve().

    \sa resolve()
*/
void *QLibrary::resolve( const QString &filename, const char *symb )
{
    QLibrary lib( filename );
    lib.setAutoUnload( FALSE );
    return lib.resolve( symb );
}

/*!
    Returns TRUE if the library is loaded; otherwise returns FALSE.

    \sa unload()
*/
bool QLibrary::isLoaded() const
{
    return d->pHnd != 0;
}

/*!
    Loads the library. Since resolve() always calls this function
    before resolving any symbols it is not necessary to call it
    explicitly. In some situations you might want the library loaded
    in advance, in which case you would use this function.
*/
bool QLibrary::load()
{
    return d->loadLibrary();
}

/*!
    Unloads the library and returns TRUE if the library could be
    unloaded; otherwise returns FALSE.

    This function is called by the destructor if autoUnload() is
    enabled.

    \sa resolve()
*/
bool QLibrary::unload()
{
    if ( !d->pHnd )
	return TRUE;

#if !defined(QT_NO_LIBRARY_UNLOAD)
    if ( !d->freeLibrary() ) {
# if defined(QT_DEBUG_COMPONENT)
	qWarning( "%s could not be unloaded", (const char*) QFile::encodeName(library()) );
# endif
	return FALSE;
    }

# if defined(QT_DEBUG_COMPONENT) && QT_DEBUG_COMPONENT == 2
    qWarning( "%s has been unloaded", (const char*) QFile::encodeName(library()) );
# endif
    d->pHnd = 0;
#endif
    return TRUE;
}

/*!
    Returns TRUE if the library will be automatically unloaded when
    this wrapper object is destructed; otherwise returns FALSE. The
    default is TRUE.

    \sa setAutoUnload()
*/
bool QLibrary::autoUnload() const
{
    return (bool)aunload;
}

/*!
    If \a enabled is TRUE (the default), the wrapper object is set to
    automatically unload the library upon destruction. If \a enabled
    is FALSE, the wrapper object is not unloaded unless you explicitly
    call unload().

    \sa autoUnload()
*/
void QLibrary::setAutoUnload( bool enabled )
{
    aunload = enabled;
}

/*!
    Returns the filename of the shared library this QLibrary object
    handles, including the platform specific file extension.

    For example:
    \code
    QLibrary lib( "mylib" );
    QString str = lib.library();
    \endcode
    will set \e str to "mylib.dll" on Windows, and "libmylib.so" on Linux.
*/
QString QLibrary::library() const
{
    if ( libfile.isEmpty() )
	return libfile;

    QString filename = libfile;

#if defined(Q_WS_WIN)
    if ( filename.findRev( '.' ) <= filename.findRev( '/' ) )
	filename += ".dll";
#else
#ifdef Q_OS_MACX
    QString filter = ".dylib";
#elif defined(Q_OS_HPUX)
    QString filter = ".sl";
#else
    QString filter = ".so";
#endif
    if ( filename.find(filter) == -1 ) {
	if(QFile::exists(filename + filter)) {
	    filename += filter;
	} else { 
	    const int x = filename.findRev( "/" );
	    if ( x != -1 ) {
		QString path = filename.left( x + 1 );
		QString file = filename.right( filename.length() - x - 1 );
		filename = QString( "%1lib%2%3" ).arg( path ).arg( file ).arg( filter );
	    } else {
		filename = QString( "lib%1%2" ).arg( filename ).arg( filter );
	    }
	}
    }
#endif

    return filename;
}
#endif //QT_NO_LIBRARY
