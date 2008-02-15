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

#ifndef _ID3TAG_H
#define _ID3TAG_H

#include "id3frame.h"
#include <QList>
#include <QPair>
#include <QDataStream>
#include <QBuffer>

class Id3Tag
{
public:
    enum TagHeaderFlag
    {
        Unsynchronisation = 0x80,
        HasExtendedHeader = 0x40,
        Experimental = 0x20,
        HasFooter = 0x10
    };

    enum TagExtendedHeaderFlag
    {
        Update = 0x40,
        Crc = 0x20,
        Restrictions = 0x10
    };

    enum TagRestrictionFlag
    {
        TagSizeMask = 0xC0,
        TagSize128_1024 = 0x00,
        TagSize64_128 = 0x40,
        TagSize32_40 =  0x80,
        TagSize32_4 = 0xC0,

        TextEncodingUtf8 = 0x20,

        TextLengthMask = 0x18,
        TextLength1024 = 0x08,
        TextLength128  = 0x10,
        TextLength30   = 0x18,

        ImageEncodingPngJpeg = 0x04,

        ImageSizeMask = 0x03,
        ImageSizeLe256 = 0x01,
        ImageSizeLe64 = 0x02,
        ImageSizeEq64 = 0x03
    };

    struct Header
    {
        char id[ 3 ];
        union
        {
            quint16 version;
            struct
            {
                quint8 minorVersion;
                quint8 majorVersion;
            };
        };
        quint8 flags;
        quint32 size;
    };

    Id3Tag( QIODevice * );
    ~Id3Tag();

    quint32 size() const;

    bool isValid() const;

    Id3Frame *readFrame( quint64 position );

    QList< QPair< quint32, qint64 > > framePositions() const;

    static quint32 convertSyncSafeInteger( quint32 syncSafe );

    static QByteArray unsynchronise( const QByteArray &data );

private:
    void readExtendedHeaders();
    void readFrameHeaders();

    Header header;

    QList< QPair< quint32, qint64 > > frames;

    qint64 offset;

    QDataStream stream;

    QBuffer tagBuffer;

    bool unsynchroniseFrames;
};

#endif
