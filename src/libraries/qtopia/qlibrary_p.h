/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
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

#include "qlibrary.h"

//#ifndef QT_NO_COMPONENT

//#ifndef QT_H
#include "qwindowdefs.h"
//#ifndef QT_LITE_COMPONENT
#include "qobject.h"
//#endif
//#endif // QT_H

class QTimer;
class QLibrary;
class QLibraryInterface;

/*
  Private helper class that saves the platform dependent handle
  and does the unload magic using a QTimer.
*/
//#ifndef QT_LITE_COMPONENT
class QTOPIA_EXPORT QLibraryPrivate : public QObject
{
    Q_OBJECT
public:
    QLibraryPrivate( QLibrary *lib );
    ~QLibraryPrivate();
 
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
//#endif // QT_LITE_COMPONENT

//#endif // QT_NO_COMPONENT

#endif // QLIBRARY_P_H
