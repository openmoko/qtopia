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

#ifndef __QTOPIA_MEDIALIBRARY_QMEDIACODECREP_H
#define __QTOPIA_MEDIALIBRARY_QMEDIACODECREP_H

#include <qstring.h>
#include <qvariant.h>

#include "qmediacodecinfo.h"


class QMediaCodecRepPrivate;

class QTOPIAMEDIA_EXPORT QMediaCodecRep
{
public:
    QMediaCodecRep();
    explicit QMediaCodecRep(QMediaCodecInfo const& codecInfo);
    ~QMediaCodecRep();

    QString id() const;

    void setValue(QString const& name, QVariant const& value);
    QVariant value(QString const& name) const;

    // {{{ Serialization
    template <typename S> void serialize(S& stream) const;
    template <typename S> void deserialize(S& stream);
    // }}}

private:
    QMediaCodecRepPrivate*  d;
};

Q_DECLARE_USER_METATYPE(QMediaCodecRep);

#endif  // __QTOPIA_MEDIALIBRARY_QMEDIACODECREP_H
