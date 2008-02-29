/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __QMAILID_H
#define __QMAILID_H

#include <QString>
#include <QVariant>
#include <qtopiaglobal.h>
#include <QSharedData>
#include <qtopiaipcmarshal.h>

class QMailIdPrivate;

class QTOPIAMAIL_EXPORT QMailId
{
public:
    QMailId();
	explicit QMailId(const quint64& id);
    QMailId(const QMailId& other);
    virtual ~QMailId();

    bool isValid() const;
    
	operator QVariant() const;

    bool operator!=( const QMailId& other ) const;
    bool operator==( const QMailId& other ) const;
    QMailId& operator=(const QMailId& other);
	bool operator <(const QMailId& other) const;

	template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

	quint64 toULongLong() const;

private:
    QSharedDataPointer<QMailIdPrivate> d;

};

Q_DECLARE_USER_METATYPE(QMailId);

typedef QList<QMailId> QMailIdList;

#endif //QMAILID_H
