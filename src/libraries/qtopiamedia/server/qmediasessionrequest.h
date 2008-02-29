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

#ifndef __QTOPIA_MEDIA_QMEDIASESSIONREQUEST_H
#define __QTOPIA_MEDIA_QMEDIaSESSIONREQUEST_H


#include <QObject>
#include <QUrl>
#include <QVariant>

#include <qtopiaglobal.h>
#include <qtopiaipcmarshal.h>


class QMediaSessionRequestPrivate;

class QTOPIAMEDIA_EXPORT QMediaSessionRequest
{
public:
    QMediaSessionRequest();
    QMediaSessionRequest(QString const& domain, QString const& type);
    QMediaSessionRequest(QMediaSessionRequest const& copy);
    ~QMediaSessionRequest();

    QUuid const& id() const;
    QString const& domain() const;
    QString const& type() const;

    QMediaSessionRequest& operator=(QMediaSessionRequest const& rhs);

    template <typename DataType>
    QMediaSessionRequest& operator<<(DataType const& data)
    {
        enqueueRequestData(qVariantFromValue(data));
        return *this;
    }

    template <typename DataType>
    QMediaSessionRequest& operator>>(DataType& data)
    {
        data = qVariantValue<DataType>(dequeueRequestData());
        return *this;
    }

    // {{{ Serialization
    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);
    // }}}

private:
    void enqueueRequestData(QVariant const& data);
    QVariant dequeueRequestData();

    QMediaSessionRequestPrivate*  d;
};

Q_DECLARE_USER_METATYPE(QMediaSessionRequest);

#endif  // __QTOPIA_MEDIA_QMEDIASESSIONREQUEST_H
