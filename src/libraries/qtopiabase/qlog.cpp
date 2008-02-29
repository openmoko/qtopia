/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QString>
#include <QSettings>
#include <stdio.h>
#include "qlog.h"

// XXX belongs in $QTDIR/doc/src/debug.qdoc
/*
    The qLog(CategoryIdentifier) function allows categorized logging.

    The syntax for logging is:

    \code
        qLog(I18n) << "Language loaded:" << lang;
    \endcode

    The categories are declared by use of one of these macros:

    \list
    \o QLOG_DISABLE(CategoryIdentifier) - no logging code is generated.
    \o QLOG_ENABLE(CategoryIdentifier) - logging code is always generated and invoked.
    \o QLOG_OPTION(CategoryIdentifier,expr) - logging code is always generated, invoked
            if \i expr is true at the time the log is issued. \i expr maty be cached.
    \o QLOG_OPTION_VOLATILE(CategoryIdentifier,expr) - as above, but \i expr is not cached.
    \o QLOG_DISABLE_UNCATEGORIZED - no logging code is generated for
            the empty category (qLog() << values).
    \endlist

    The CategoryIdentifier has _QLog appended within the macros,
    so it can be any identifier you choose. The associated type name
    might be a useful choice:

    \code
        qLog(QWidget) << "Created" << name;
    \endcode

    Example mechanisms by which your \i expr might choose whether logging is enabled include:
    \list
    \o Look up the CategoryIdentifier as a key in QSettings.
    \o Return a field from a global record that is set at startup.
    \endlist
*/

QTOPIABASE_EXPORT QDebug QLogBase::log(const char* category)
{
    QDebug r = QDebug(QtDebugMsg);
    if ( category )
        r << category << ": ";
    return r;
}
