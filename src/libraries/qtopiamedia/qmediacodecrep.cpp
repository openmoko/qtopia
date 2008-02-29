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

#include "qmediacodecrep.h"


typedef QMap<QString, QVariant> Options;

class QMediaCodecRepPrivate
{
public:
    QString      id;
    Options      options;
};


QMediaCodecRep::QMediaCodecRep()
{
}

QMediaCodecRep::QMediaCodecRep(QMediaCodecInfo const& codecInfo):
    d(new QMediaCodecRepPrivate)
{
    d->id = codecInfo.name();
}


QMediaCodecRep::~QMediaCodecRep()
{
    delete d;
}

QString QMediaCodecRep::id() const
{
    return d->id;
}

void QMediaCodecRep::setValue(QString const& name, QVariant const& value)
{
    d->options[name] = value;
}

QVariant QMediaCodecRep::value(QString const& name) const
{
    Options::iterator it = d->options.find(name);
    if (it != d->options.end())
        return *it;

    return QVariant();
}

// {{{ Serialization
template <typename S>
void QMediaCodecRep::serialize(S& stream) const
{
    stream << d->id;
    stream << d->options;
}

template <typename S>
void QMediaCodecRep::deserialize(S& stream)
{
    stream >> d->id;
    stream >> d->options;
}
// }}}

Q_IMPLEMENT_USER_METATYPE(QMediaCodecRep);


