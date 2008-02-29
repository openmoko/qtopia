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

#include "qmediadevicerep.h"


class QMediaDeviceRepPrivate
{
public:
    QString      id;
    QMediaDeviceRep::Options      options;
};

QMediaDeviceRep::QMediaDeviceRep()
{
}

QMediaDeviceRep::QMediaDeviceRep(QMediaDeviceInfo const& deviceInfo):
    d(new QMediaDeviceRepPrivate)
{
    d->id = deviceInfo.name();
}


QMediaDeviceRep::~QMediaDeviceRep()
{
    delete d;
}

QString QMediaDeviceRep::id() const
{
    return d->id;
}

void QMediaDeviceRep::setValue(QString const& name, QVariant const& value)
{
    d->options[name] = value;
}

QVariant QMediaDeviceRep::value(QString const& name) const
{
    Options::iterator it = d->options.find(name);
    if (it != d->options.end())
        return *it;

    return QVariant();
}

QMediaDeviceRep::Options const& QMediaDeviceRep::options() const
{
    return d->options;
}

// {{{ Serialization
template <typename S>
void QMediaDeviceRep::serialize(S& stream) const
{
    stream << d->id;
    stream << d->options;
}

template <typename S>
void QMediaDeviceRep::deserialize(S& stream)
{
    stream >> d->id;
    stream >> d->options;
}
// }}}

Q_IMPLEMENT_USER_METATYPE(QMediaDeviceRep);



