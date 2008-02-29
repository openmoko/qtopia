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
#include <private/qobexauthentication_p.h>

#include <QFile>
#include <QCryptographicHash>
#include <QDateTime>

/*!
    \internal
    Stores a randomly generated nonce into \a buf.
*/
void QObexAuth::generateNonce(QByteArray &buf)
{
    QFile f("/dev/urandom");
    f.open(QFile::ReadOnly);
    QByteArray random(f.read(16));
    f.close();

    buf = QCryptographicHash::hash(
        random + ':' + QByteArray::number(QDateTime::currentDateTime().toTime_t()),
        QCryptographicHash::Md5);
}
