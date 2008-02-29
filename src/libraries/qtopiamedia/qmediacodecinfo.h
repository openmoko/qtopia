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

#ifndef __QTOPIA_MEDIALIBRARY_MEDIACODECINFO_H
#define __QTOPIA_MEDIALIBRARY_MEDIACODECINFO_H

#include <qstring.h>

#include <qtopiaipcmarshal.h>

class QTOPIAMEDIA_EXPORT QMediaCodecInfo
{
    struct QMediaCodecInfoPrivate;

public:
    QMediaCodecInfo();
    QMediaCodecInfo(QMediaCodecInfo const& codec);
    ~QMediaCodecInfo();

    QMediaCodecInfo& operator=(QMediaCodecInfo const& rhs);
    bool operator==(QMediaCodecInfo const& rhs);

    QString id() const;

    QString name() const;
    QString comment() const;
    QString mimeType() const;

    double version() const;

    bool canEncode() const;
    bool canDecode() const;

    bool supportsVideo() const;
    bool supportsAudio() const;

    // {{{ Serialization
    template <typename S> void serialize(S& stream) const;
    template <typename S> void deserialize(S& stream);
    // }}}

private:
    QMediaCodecInfoPrivate* d;
};

typedef QList<QMediaCodecInfo>  QMediaCodecInfoList;

Q_DECLARE_USER_METATYPE(QMediaCodecInfo);

#endif  // __QTOPIA_MEDIALIBRARY_MEDIACODECINFO_H
