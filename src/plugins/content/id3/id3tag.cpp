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

#include "id3tag.h"
#include <QtDebug>

/*!
    \class Id3Tag
    \internal
*/
Id3Tag::Id3Tag( QIODevice *device )
    : stream( device )
{
    stream.setByteOrder( QDataStream::BigEndian );

    stream.readRawData( header.id, 3 );

    stream >> header.version;
    stream >> header.flags;
    stream >> header.size;

    header.size = convertSyncSafeInteger( header.size );

    if( !isValid() )
        return;

    if( header.majorVersion == 3 && header.flags & Unsynchronisation )
    {
        QByteArray tagData = unsynchronise( device->read( header.size ) );

        tagBuffer.setData( tagData );

        if( !tagBuffer.open( QIODevice::ReadOnly ) )
            return;

        stream.setDevice( &tagBuffer );
    }

    offset = device->pos();

    unsynchroniseFrames = header.majorVersion == 4 && header.flags & Unsynchronisation;

    if( header.flags & HasExtendedHeader )
        readExtendedHeaders();

    readFrameHeaders();
}

Id3Tag::~Id3Tag()
{
    if( tagBuffer.isOpen() )
        tagBuffer.close();
}

quint32 Id3Tag::size() const
{
    return header.size;
}

bool Id3Tag::isValid() const
{
    return header.id[ 0 ] == 'I' &&
           header.id[ 1 ] == 'D' &&
           header.id[ 2 ] == '3' &&
           header.size > 0       &&
           header.majorVersion >= 2 &&
           header.majorVersion <= 4;
}

Id3Frame *Id3Tag::readFrame( quint64 position )
{
    stream.device()->seek( position );

    Id3Frame::Header frameHeader = Id3Frame::readHeader( stream, header.version );

    QByteArray data = stream.device()->read( frameHeader.size );

    return new Id3Frame( frameHeader, data, header.majorVersion, unsynchroniseFrames );
}

QList< QPair< quint32, qint64 > > Id3Tag::framePositions() const
{
    return frames;
}

void Id3Tag::readExtendedHeaders()
{
    qint64 pos = stream.device()->pos();

    quint32 size;

    stream >> size;

    if( header.majorVersion == 4 )
        size = convertSyncSafeInteger( size );

    stream.device()->seek( pos + size );
}

void Id3Tag::readFrameHeaders()
{
    while( stream.device()->pos() < offset + header.size )
    {
        qint64 pos = stream.device()->pos();

        Id3Frame::Header frameHeader = Id3Frame::readHeader( stream, header.version );

        if( frameHeader.id == 0 )
            return;

        frames.append( QPair< quint32, qint64 >( frameHeader.id, pos ) );

        stream.device()->seek( stream.device()->pos() + frameHeader.size );

    }
}

quint32 Id3Tag::convertSyncSafeInteger( quint32 syncSafe )
{
    return ((syncSafe & 0x7F000000) >> 0x03) |
           ((syncSafe & 0x007F0000) >> 0x02) |
           ((syncSafe & 0x00007F00) >> 0x01) |
           ((syncSafe & 0x0000007F)        );
}

QByteArray Id3Tag::unsynchronise( const QByteArray &data )
{
    if( data.size() == 0 )
        return data;

    int shift = 0;

    QByteArray unsynced = data;

    for( int i = 1; i < data.size(); i++ )
    {
        if( data[ i - 1 ] == (char)0xFF && data[ i ] == (char)0x00 )
            shift++;
        else if( shift )
            unsynced[ i - shift ] = data[ i ];
    }

    if( shift )
        unsynced.chop( shift );

    return unsynced;
}

