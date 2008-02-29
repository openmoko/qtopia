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

#include "id3frame.h"
#include "id3tag.h"
#include <QBuffer>
#include <QTextStream>
#include <QtDebug>

/*!
    \class Id3Frame
    \internal
 */

Id3Frame::Id3Frame( const Header &h, const QByteArray &d, quint8 id3MajorVersion, bool tagUnsynchronisation )
    : header( h )
    , data( d )
    , majorVersion( id3MajorVersion )
{
    if( header.flags & Unsynchronisation || tagUnsynchronisation )
        data = Id3Tag::unsynchronise( data );

    if( header.flags & Compression )
        data = qUncompress( data );

    if( header.flags & DataLengthIndicator && data.size() >= 4 )
    {
        {
            QDataStream stream( data );

            stream.setByteOrder( QDataStream::BigEndian );

            stream >> header.size;

            header.size = Id3Tag::convertSyncSafeInteger( header.size );
        }

        data = data.right( data.size() - 4 );
    }
}

Id3Frame::~Id3Frame()
{

}

bool Id3Frame::isTextFrame() const
{
    return header.idBytes[ 0 ] == 'T';
}

quint32 Id3Frame::id() const
{
    return header.id;
}

QStringList Id3Frame::textFields()
{
    QStringList strings;

    if( header.idBytes[ 0 ] == 'T' )
    {
        QBuffer buffer( &data );

        buffer.open( QIODevice::ReadOnly );

        char encoding = buffer.read( 1 )[ 0 ];

        QTextStream textStream( &buffer );

        switch( encoding )
        {
            case 0x00:
                textStream.setCodec( "ISO 8859-1" );
                break;
            case 0x01:
                textStream.setCodec( "UTF-16" );
                break;
            case 0x02:
                textStream.setCodec( "UTF-16E" );
                break;
            case 0x03:
                textStream.setCodec( "UTF-8" );
        }

        QString all = textStream.readAll();

        if( all.endsWith( QChar( '\0' ) ) )
            all.chop( 1 );

        return all.split( majorVersion == 2 ? '\\' : '\0' );
    }

    return QStringList();
}

Id3Frame::Header Id3Frame::readHeader( QDataStream &stream, quint32 version )
{
    Id3Frame::Header header;

    stream.readRawData( header.idBytes, 4 );

    if( (version & 0xFF00) == 0x0200 )
    {
        quint16 size;

        stream >> size;

        header.size = size;
        header.flags = 0;
    }
    else
    {
        stream >> header.size;
        stream >> header.flags;

        // Header size should be a syncsafe integer according to Id3v2.4 spec
        // but at least iTunes does not encode it as such.
//         if( (header.version & 0xFF00) == 0x0400 )
//             frameHeader.size = convertSyncSafeInteger( frameHeader.size );
    }

    return header;
}

