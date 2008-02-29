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
#include "md5file.h"
#include <stdlib.h>
#include <QtDebug>

Md5File::Md5File( QObject *parent )
    : QFile( parent )
{
}

Md5File::Md5File( const QString &fileName, QObject *parent )
    : QFile( fileName, parent )
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
        MD5Init( &m_context);

        return true;
    }
    else
        return false;
}

void Md5File::close()
{
    QFile::close();

    uchar digest[ 17 ];

    MD5Final( digest, &m_context );

    QByteArray md5Sum;

    md5Sum.resize( 32 );
    char *hexData = md5Sum.data();

    for (int i = 0; i < 16; ++i) {
        int j = (digest[i] >> 4) & 0xf;
        if (j <= 9)
            hexData[i*2] = (j + '0');
        else
            hexData[i*2] = (j + 'a' - 10);
        j = digest[i] & 0xf;
        if (j <= 9)
            hexData[i*2+1] = (j + '0');
        else
            hexData[i*2+1] = (j + 'a' - 10);
    }

    m_md5Sum = md5Sum;
}

qint64 Md5File::writeData( const char *data, qint64 maxSize )
{
    MD5Update( &m_context, reinterpret_cast< const md5byte * >( data ), maxSize );

    return QFile::writeData( data, maxSize );
}
