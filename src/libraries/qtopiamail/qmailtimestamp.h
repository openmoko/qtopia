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
#ifndef QMAILTIMESTAMP_H
#define QMAILTIMESTAMP_H

#include <qtopiaglobal.h>

#include <QDateTime>
#include <QSharedDataPointer>
#include <QString>

class QMailTimeStampPrivate;

class QTOPIAMAIL_EXPORT QMailTimeStamp
{
public:
    static QMailTimeStamp currentDateTime();

    QMailTimeStamp();
    explicit QMailTimeStamp(const QString& timeText);
    explicit QMailTimeStamp(const QDateTime& dateTime);
    QMailTimeStamp(const QMailTimeStamp& other);
    ~QMailTimeStamp();

    QString toString() const;

    QDateTime toLocalTime() const;
    QDateTime toUTC() const;

    bool isNull() const;
    bool isValid() const;

    bool operator== (const QMailTimeStamp& other) const;
    bool operator!= (const QMailTimeStamp& other) const;

    bool operator< (const QMailTimeStamp& other) const;
    bool operator<= (const QMailTimeStamp& other) const;

    bool operator> (const QMailTimeStamp& other) const;
    bool operator>= (const QMailTimeStamp& other) const;

    const QMailTimeStamp& operator=(const QMailTimeStamp& other);

private:
    QSharedDataPointer<QMailTimeStampPrivate> d;
};

#endif // QMAILTIMESTAMP_H

