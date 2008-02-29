/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "qmediadeviceinfo.h"

struct QMediaDeviceInfo::QMediaDeviceInfoPrivate
{
    QString id;
    QString name;
    bool    isReadable;
    bool    isWritable;
};

QMediaDeviceInfo::QMediaDeviceInfo():
    d(new QMediaDeviceInfoPrivate)
{
}

QMediaDeviceInfo::QMediaDeviceInfo(QMediaDeviceInfo const& info):
    d(new QMediaDeviceInfoPrivate)
{
    *d = *info.d;
}

QMediaDeviceInfo::~QMediaDeviceInfo()
{
    delete d;
}

QMediaDeviceInfo& QMediaDeviceInfo::operator=(QMediaDeviceInfo const& rhs)
{
    *d = *rhs.d;
    return *this;
}

bool QMediaDeviceInfo::operator==(QMediaDeviceInfo const& rhs)
{
    return d->id == rhs.d->id;
}

QString QMediaDeviceInfo::id() const
{
    return d->id;
}

QString QMediaDeviceInfo::name() const
{
    return d->name;
}

bool QMediaDeviceInfo::isReadable() const
{
    return d->isReadable;
}

bool QMediaDeviceInfo::isWritable() const
{
    return d->isWritable;
}

// {{{ Serialization
template <typename Stream>
void QMediaDeviceInfo::serialize(Stream &stream) const
{
    stream << d->id;
    stream << d->name;
    stream << d->isReadable;
    stream << d->isWritable;
}

template <typename Stream>
void QMediaDeviceInfo::deserialize(Stream &stream)
{
    stream >> d->id;
    stream >> d->name;
    stream >> d->isReadable;
    stream >> d->isWritable;
}
// }}}

Q_IMPLEMENT_USER_METATYPE(QMediaDeviceInfo);


