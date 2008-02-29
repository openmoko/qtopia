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
#ifndef QMAILADDRESS_H
#define QMAILADDRESS_H

#include <qtopiaglobal.h>

#include <QContact>
#include <QList>
#include <QSharedDataPointer>
#include <QString>
#include <QStringList>

class QMailAddressPrivate;

class QTOPIAMAIL_EXPORT QMailAddress
{
public:
    QMailAddress();
    explicit QMailAddress(const QString& addressText);
    QMailAddress(const QString& name, const QString& emailAddress);
    QMailAddress(const QMailAddress& other);
    ~QMailAddress();

    bool isNull() const;

    QString name() const;
    QString address() const;
    QString displayName() const;
    QString displayName(QContactModel& fromModel) const;

    bool isGroup() const;
    QList<QMailAddress> groupMembers() const;

    QContact matchContact() const;
    bool matchesExistingContact() const;
    QContact matchContact(QContactModel& fromModel) const;

    bool isPhoneNumber() const;
    bool isEmailAddress() const;

    QString minimalPhoneNumber() const;

    QString toString() const;

    bool operator==(const QMailAddress& other) const;

    const QMailAddress& operator=(const QMailAddress& other);

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    static QStringList toStringList(const QList<QMailAddress>& list);
    static QList<QMailAddress> fromStringList(const QString& list);
    static QList<QMailAddress> fromStringList(const QStringList& list);

    static QString removeComments(const QString& input);
    static QString removeWhitespace(const QString& input);

    static QString phoneNumberPattern();
    static QString emailAddressPattern();

private:
    QSharedDataPointer<QMailAddressPrivate> d;
};

Q_DECLARE_USER_METATYPE(QMailAddress)

typedef QList<QMailAddress> QMailAddressList;

Q_DECLARE_METATYPE(QMailAddressList);
Q_DECLARE_USER_METATYPE_TYPEDEF(QMailAddressList, QMailAddressList);

#endif // QMAILADDRESS_H 

