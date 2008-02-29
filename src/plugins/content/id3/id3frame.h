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

#ifndef _ID3FRAME_H
#define _ID3FRAME_H

#include <QtGlobal>
#include <QByteArray>
#include <QStringList>

class Id3Frame
{
public:

    struct Header
    {
        union
        {
            quint32 id;
            char idBytes[ 4 ];
        };
        quint32 size;
        quint16 flags;
    };

    enum FrameFlag
    {
        TagAlter = 0x4000,
        FileAlter = 0x0200,
        ReadOnly = 0x0100,

        Grouping = 0x0040,

        Compression         = 0x0008,
        Encryption          = 0x0004,
        Unsynchronisation   = 0x0002,
        DataLengthIndicator = 0x0001
    };

    Id3Frame( const Header &h, const QByteArray &d, quint8 Id3MajorVersion, bool tagUnsynchronisation );
    ~Id3Frame();

    bool isTextFrame() const;

    quint32 id() const;

    QStringList textFields();

    static Header readHeader( QDataStream &stream, quint32 version );

private:

    void unsynchronise();

    Header header;
    QByteArray data;
    quint8 majorVersion;
};

#endif
