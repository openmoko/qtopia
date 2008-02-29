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
#include "md5file.h"
#include <stdlib.h>
#include <QtDebug>

Md5File::Md5File( QObject *parent )
    : QFile( parent )
    , m_hash( QCryptographicHash::Md5 )
{
}

Md5File::Md5File( const QString &fileName, QObject *parent )
    : QFile( fileName, parent )
    , m_hash( QCryptographicHash::Md5 )
{
}

QString Md5File::md5Sum() const
{
    return m_md5Sum;
}

bool Md5File::open( OpenMode mode )
{
    if( QFile::open( mode ) )
    {
        m_hash.reset();

        return true;
    }
    else
        return false;
}

void Md5File::close()
{
    QFile::close();

    m_md5Sum = m_hash.result().toHex();
}

qint64 Md5File::writeData( const char *data, qint64 maxSize )
{
    m_hash.addData( data, maxSize );

    return QFile::writeData( data, maxSize );
}
