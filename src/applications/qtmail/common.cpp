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

#include "common.h"
#include <QString>

// just grabbed from example mail program, rememeber to test...

void QtMail::replace( QString& str, const QString& before, const QString& after )
{
    if ( before.length() == 0 )
        return;

    int maxsize;
    if ( before.length() > after.length() )
        maxsize = str.length();
    else
        maxsize = ( str.length() / before.length() ) * after.length();

    QChar *buf = new QChar[maxsize + 1]; // + 1 in case maxsize is 0
    const QChar *strBuf = str.unicode();

    int prev = 0;
    int cur = 0;
    int i = 0;
    bool changed = false;

    // indexOf may be -1
    while ( (uint) (cur = str.indexOf(before, prev)) < (uint) str.length() ) {
        if ( cur > prev ) {
            memcpy( buf + i, strBuf + prev, sizeof(QChar) * (cur - prev) );
            i += cur - prev;
}
        if ( after.length() > 0 ) {
            memcpy( buf + i, after.unicode(), sizeof(QChar) * after.length() );
            i += after.length();
}
        prev = cur + before.length();
        changed = true;
}

    if ( changed ) {
        memcpy( buf + i, strBuf + prev,
        sizeof(QChar) * (str.length() - prev) );
        i += str.length() - prev;
        str = QString( buf, i );
}
    delete[] buf;
}
