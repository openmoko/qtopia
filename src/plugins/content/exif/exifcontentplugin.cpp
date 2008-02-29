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

#include "exifcontentplugin.h"
#include "ifd.h"
#include <QtDebug>
#include <qmimetype.h>

/*!
    \class ExifContentPlugin
    \internal

    Plugin for reading meta data from images with exif tags.
*/

ExifContentPlugin::ExifContentPlugin()
{
}

ExifContentPlugin::~ExifContentPlugin()
{
}

QStringList ExifContentPlugin::keys() const
{
    return QMimeType( QLatin1String( "image/tiff" ) ).extensions() +
           QMimeType( QLatin1String( "image/jpeg" ) ).extensions();
}

bool ExifContentPlugin::installContent( const QString &filePath, QContent *content )
{
    QFile file( filePath );

    bool success = false;

    if( file.open( QIODevice::ReadOnly ) )
    {
        if( readFile( &file, content ) )
        {
            content->setName( QFileInfo( filePath ).baseName() );
            content->setType( QMimeType( filePath ).id() );
            content->setFile( filePath );

            success = true;
        }

        file.close();
    }

    return success;
}

bool ExifContentPlugin::updateContent( QContent *content )
{
    return installContent( content->fileName(), content );
}

bool ExifContentPlugin::readFile( QFile *file, QContent *content )
{
    if( file->peek( 4 ) == "\xFF\xD8\xFF\xE1" )
    {
        file->seek( 6 );

        if( file->read( 4 ) != "Exif" )
            return false;

        file->seek( 12 );
    }

    qint64 baseOffset = file->pos();

    QByteArray byteOrder = file->read( 2 );

    QDataStream stream( file );

    if( byteOrder == "II" )
        stream.setByteOrder( QDataStream::LittleEndian );
    else if( byteOrder == "MM" )
        stream.setByteOrder( QDataStream::BigEndian );
    else
        return false;

    quint16 id;
    quint32 ifdOffset;

    stream >> id;
    stream >> ifdOffset;

    if( id != 0x002A )
        return false;

    file->seek( baseOffset + ifdOffset );

    readProperties( stream, baseOffset, content );

    return true;
}

QList< Ifd::Header > ExifContentPlugin::readIfdHeaders( QDataStream &stream, int baseOffset )
{
    QList< Ifd::Header > headers;

    while( true && !stream.device()->atEnd() )
    {
        quint16 headerCount;

        stream >> headerCount;

        for( quint32 i = 0; i < headerCount; i++ )
        {
            Ifd::Header header;

            stream >> header;

            headers.append( header );
        }

        quint32 ifdOffset;

        stream >> ifdOffset;

        if( ifdOffset )
            stream.device()->seek( baseOffset + ifdOffset );
        else
            break;
    }

    return headers;
}

void ExifContentPlugin::readProperties( QDataStream &stream, int baseOffset, QContent *content )
{
    QList< Ifd::Header > headers = readIfdHeaders( stream, baseOffset );

    foreach( Ifd::Header header, headers )
    {
        if( header.tag == Ifd::DateTime && header.type == Ifd::Ascii )
        {
            Ifd ifd( header, const_cast< QDataStream & >(stream), baseOffset );

            content->setProperty( QLatin1String( "CreationDate" ), ifd.toString(), QLatin1String( "Image" ) );
        }
        if( header.tag == Ifd::ImageDescription && header.type == Ifd::Ascii )
        {
            Ifd ifd( header, const_cast< QDataStream & >(stream), baseOffset );

            content->setProperty( QContent::Description, ifd.toString() );
        }
        if( header.tag == Ifd::Artist && header.type == Ifd::Ascii )
        {
            Ifd ifd( header, const_cast< QDataStream & >(stream), baseOffset );

            content->setProperty( QContent::Artist, ifd.toString() );
        }
    }
}

QTOPIA_EXPORT_PLUGIN(ExifContentPlugin);
