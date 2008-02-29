/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "md5hash.h"

#include <QCryptographicHash>
#include <QString>

#include <stdlib.h>

QString MD5::hash( const QString &str )
{
    char *inbuf;
    int inbufSize = str.length() * sizeof(QChar);
    inbuf = (char *)::malloc( inbufSize );
    Q_CHECK_PTR( inbuf );
    ::memcpy( inbuf, str.unicode(), inbufSize );

    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(inbuf, inbufSize);

    QByteArray digest = hash.result();
    return QString::fromLatin1(digest.toHex());
}
