/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qnamespace.h>
#if !(QT_VERSION-0 < 0x030000)
#include <qlibrary.h>
#include <private/qcom_p.h>
#include <private/qcomlibrary_p.h>
#else

#ifndef QLIBRARY_H
#define QLIBRARY_H

#include <qtopia/qpeglobal.h>
#include <qstring.h>
#include <qdict.h>

#ifndef QT_NO_COMPONENT

#include <qtopia/qcom.h>

#ifndef QTOPIA_FAKE_COMPONENT
class QLibraryPrivate;
#else
#include <qdict.h>
#endif

class QTOPIA_EXPORT QLibrary
{
public:
    enum Policy
    {
	Delayed,
	Immediately,
	Manual
    };

    QLibrary( const QString& filename, Policy = Delayed );
    ~QLibrary();

#ifndef QTOPIA_FAKE_COMPONENT
    void *resolve( const char* );
    static void *resolve( const QString &filename, const char * );
#endif

    bool unload( bool force = FALSE );
    bool isLoaded() const;

    void setPolicy( Policy pol );
    Policy policy() const;

    QString library() const;

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );

private:
    void createInstanceInternal();
#ifndef QTOPIA_FAKE_COMPONENT
    QLibraryPrivate *d;
#endif

    bool load();

    QString libfile;
    Policy libPol;
    QUnknownInterface *entry;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLibrary( const QLibrary & );
    QLibrary &operator=( const QLibrary & );
#endif
};

#endif // QT_NO_COMPONENT

#define Q_DEFINED_QLIBRARY
#include <qtopia/qtopiawinexport.h>
#endif //QLIBRARY_H

#endif // Qt < 3.0
