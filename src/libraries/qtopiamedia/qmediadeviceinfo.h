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

#ifndef __QTOPIA_MEDIALIBRARY_MEDIADEVICEINFO_H
#define __QTOPIA_MEDIALIBRARY_MEDIADEVICEINFO_H

#include <qstring.h>
#include <qlist.h>
#include <qvariant.h>

#include <qtopiaipcmarshal.h>

class QTOPIAMEDIA_EXPORT QMediaDeviceInfo
{
    struct QMediaDeviceInfoPrivate;

public:
    QMediaDeviceInfo();
    QMediaDeviceInfo(QMediaDeviceInfo const& info);
    ~QMediaDeviceInfo();

    QMediaDeviceInfo& operator=(QMediaDeviceInfo const& rhs);
    bool operator==(QMediaDeviceInfo const& rhs);

    QString id() const;

    QString name() const;

    bool isReadable() const;
    bool isWritable() const;

    // {{{ Serialization
    template <typename Stream>
    void serialize(Stream &stream) const;
    template <typename Stream>
    void deserialize(Stream &stream);
    // }}}

private:
    QMediaDeviceInfoPrivate*    d;
};

typedef QList<QMediaDeviceInfo>     QMediaDeviceInfoList;

Q_DECLARE_USER_METATYPE(QMediaDeviceInfo);

#endif  // __QTOPIA_MEDIALIBRARY_MEDIADEVICEINFO_H
