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

#ifndef QLIBRARY_P_H
#define QLIBRARY_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//

#include <qtopia/qlibrary.h>
#ifdef Q_OS_WIN32
#include <windows.h>
#include <winbase.h>
#endif

//#ifndef QT_NO_COMPONENT

//#ifndef QT_H
#include <qwindowdefs.h>
//#ifndef QT_LITE_COMPONENT
#include <qobject.h>
//#endif
//#endif // QT_H

class QTimer;
class QLibrary;
struct QLibraryInterface;

/*
  Private helper class that saves the platform dependent handle
  and does the unload magic using a QTimer.
*/
#ifndef QT_LITE_COMPONENT
class QTOPIA_EXPORT QLibraryPrivate : public QObject
{
    Q_OBJECT
public:
    QLibraryPrivate( QLibrary *lib );
    ~QLibraryPrivate();

    void startTimer();
    void killTimer();

#ifdef Q_OS_WIN32
    HINSTANCE pHnd;
#else
    void *pHnd;
#endif

    QLibraryInterface *libIface;

    bool loadLibrary();
    bool freeLibrary();
    void *resolveSymbol( const char * );

private slots:
    void tryUnload();

private:
    QTimer *unloadTimer;
    QLibrary *library;
};

#else // QT_LITE_COMPONENT
class QTOPIA_EXPORT QLibraryPrivate
{
public:
    QLibraryPrivate( QLibrary *lib );

    void startTimer();
    void killTimer();

#ifdef Q_WS_WIN
    HINSTANCE pHnd;
#else
    void *pHnd;
#endif
    QLibraryInterface *libIface;

    bool loadLibrary();
    bool freeLibrary();
    void *resolveSymbol( const char * );

private:
    QLibrary *library;
};
#endif // QT_LITE_COMPONENT

//#endif // QT_NO_COMPONENT

#endif // QLIBRARY_P_H
