/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef QPHONENUMBER_H
#define QPHONENUMBER_H

#include <qtopiaglobal.h>
#include <qobject.h>
#include <qstring.h>

class QTOPIAPIM_EXPORT QPhoneNumber
{
private:
    QPhoneNumber() {}

public:

    static QString stripNumber( const QString& number );

    static int matchNumbers( const QString& num1, const QString& num2 );

    static bool matchPrefix( const QString& num, const QString& prefix );

    static QString resolveLetters( const QString& number );

    static QString localNumber( const QString &number);
};

#endif /* QPHONENUMBER_H */
