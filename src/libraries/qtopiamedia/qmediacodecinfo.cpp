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

#include "qmediacodecinfo.h"

// {{{ QMediaCodecInfoPrivate
class QMediaCodecInfoPrivate
{
public:
    QString id;
    QString name;
    QString comment;
    QString mimeType;
    double  version;
    bool    canEncode;
    bool    canDecode;
    bool    supportsVideo;
    bool    supportsAudio;
};
// }}}

/*!
    \class QMediaCodecInfo
    \internal
*/

// {{{ QMediaCodecInfo
QMediaCodecInfo::QMediaCodecInfo():
    d(new QMediaCodecInfoPrivate)
{
}

QMediaCodecInfo::QMediaCodecInfo(QMediaCodecInfo const& codec):
    d(new QMediaCodecInfoPrivate)
{
    *d = *codec.d;
}

QMediaCodecInfo::~QMediaCodecInfo()
{
    delete d;
}

QMediaCodecInfo& QMediaCodecInfo::operator=(QMediaCodecInfo const& rhs)
{
    *d = *rhs.d;
    return *this;
}

bool QMediaCodecInfo::operator==(QMediaCodecInfo const& rhs)
{
    return d->name == rhs.d->name;     // XXX
}

QString QMediaCodecInfo::id() const
{
    return d->id;
}

QString QMediaCodecInfo::name() const
{
    return d->name;
}

QString QMediaCodecInfo::comment() const
{
    return d->comment;
}

QString QMediaCodecInfo::mimeType() const
{
    return d->mimeType;
}

double QMediaCodecInfo::version() const
{
    return d->version;
}

bool QMediaCodecInfo::canEncode() const
{
    return d->canEncode;
}

bool QMediaCodecInfo::canDecode() const
{
    return d->canDecode;
}

bool QMediaCodecInfo::supportsVideo() const
{
    return d->supportsVideo;
}

bool QMediaCodecInfo::supportsAudio() const
{
    return d->supportsAudio;
}

// {{{ Serialization
/*!
    \fn void QMediaCodecInfo::serialize(S& stream) const
    \internal
*/
template <typename S>
void QMediaCodecInfo::serialize(S& stream) const
{
    stream << d->name;
    stream << d->comment;
    stream << d->mimeType;
    stream << d->version;
    stream << d->canEncode;
    stream << d->canDecode;
    stream << d->supportsVideo;
    stream << d->supportsAudio;
}

/*!
    \fn void QMediaCodecInfo::deserialize(S& stream)
    \internal
*/
template <typename S>
void QMediaCodecInfo::deserialize(S& stream)
{
    stream >> d->name;
    stream >> d->comment;
    stream >> d->mimeType;
    stream >> d->version;
    stream >> d->canEncode;
    stream >> d->canDecode;
    stream >> d->supportsVideo;
    stream >> d->supportsAudio;
}
// }}}

// }}}

Q_IMPLEMENT_USER_METATYPE(QMediaCodecInfo);



