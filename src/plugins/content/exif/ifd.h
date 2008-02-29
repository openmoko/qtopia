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

#ifndef _IFD_H
#define _IFD_H

#include <qtopiaglobal.h>
#include <QVector>
#include <QDataStream>

class Ifd
{
public:

    enum Tag
    {
        ImageWidth       = 0x0100,
        ImageLength      = 0x0101,
        DateTime         = 0x0132,
        ImageDescription = 0x010E,
        Artist           = 0x013B,
        Make             = 0x010F,
        Model            = 0x0110,
        Software         = 0x0131
    };

    enum Type
    {
        Byte           =  1,
        Ascii          =  2,
        Short          =  3,
        Long           =  4,
        Rational       =  5,
        Undefined      =  7,
        SignedLong     =  9,
        SignedRational = 10
    };

    struct Header
    {
        quint16 tag;
        quint16 type;
        quint32 count;
        quint32 offset;
    };

    struct URational
    {
        quint32 numerator;
        quint32 denominator;
    };

    struct SRational
    {
        qint32 numerator;
        qint32 denominator;
    };

    Ifd( const Header &h, QDataStream &stream, qint64 baseOffset );
    ~Ifd();

    quint16 tag()   const;
    Type    type()  const;
    quint32 count() const;

    quint8    toByte()           const;
    quint16   toShort()          const;
    quint32   toLong()           const;
    URational toRational()       const;
    qint32    toSignedLong()     const;
    SRational toSignedRational() const;

    QByteArray           toByteArray()           const;
    QString              toString()              const;
    QVector< quint16   > toShortArray()          const;
    QVector< quint32   > toLongArray()           const;
    QVector< URational > toRationalArray()       const;
    QVector< qint32    > toSignedLongArray()     const;
    QVector< SRational > toSignedRationalArray() const;


private:

    Header header;

    union
    {
        char      *bytes;
        quint16   *uShorts;
        quint32   *uLongs;
        URational *uRationals;
        qint32    *sLongs;
        SRational *sRationals;

        quint8    byte;
        quint16   uShort;
        quint32   uLong;
        URational uRational;
        qint32    sLong;
        SRational sRational;

        quint64 base;
    };
};

QDataStream &operator >>( QDataStream &stream, Ifd::Header &header );
QDataStream &operator >>( QDataStream &stream, Ifd::URational &rational );
QDataStream &operator >>( QDataStream &stream, Ifd::SRational &rational );

#endif
