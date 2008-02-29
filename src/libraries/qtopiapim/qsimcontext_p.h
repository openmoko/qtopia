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
#ifndef _QSIMCONTEXT_P_H_
#define _QSIMCONTEXT_P_H_

#include <qtopiasql.h>
#include <qpimsource.h>
#include <qcontact.h>
#include <qphonebook.h>
#include <qcontactmodel.h>

class ContactSqlIO;
class QSimInfo;
class QContactSimContext : public QContactContext
{
    Q_OBJECT
public:
    // could have constructor protected/private with friends class.
    QContactSimContext(QObject *parent, QObject *access);

    QIcon icon() const; // default empty
    QString description() const;
    QString title() const;
    QString title(const QPimSource &) const;

    bool editable() const; // default true
    bool editable(const QUniqueId &) const; // default true

    QPimSource defaultSource() const;

    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> visibleSources() const;
    QSet<QPimSource> sources() const;
    QUuid id() const;

    bool exists(const QUniqueId &) const;
    QPimSource source(const QUniqueId &) const;

    bool updateContact(const QContact &);
    bool removeContact(const QUniqueId &);
    QUniqueId addContact(const QContact &, const QPimSource &);

    bool importContacts(const QPimSource &, const QList<QContact> &);
    QList<QContact> exportContacts(const QPimSource &, bool &) const;
    QContact exportContact(const QUniqueId &, bool &) const;

    bool waitingOnSim() const;
signals:
    void simResponded();

private slots:
    void simIdentityChanged();
    void updatePhoneBook( const QString &store, const QList<QPhoneBookEntry> &list );
    void updatePhoneBookLimits( const QString &store, const QPhoneBookLimits &value );
    void updateSqlEntries();
private:
    QContact simContact(const QUniqueId &u) const;
    QString card(const QUniqueId &) const;
    int cardIndex(const QUniqueId &) const;
    QUniqueId id(const QString &card, int index) const;
    QUniqueId findLabel(const QString &) const;

    int nextFreeIndex() const;

    QContact contact(const QPhoneBookEntry &entry) const;
    bool isSIMContactCompatible(const QContact &c) const;

    static QString typeToSIMExtension(QContactModel::Field type);
    static QContactModel::Field SIMExtensionToType(QString &label);
    QString createSIMLabel(const QContact &c);


    QList<QUuid> mCards;
    QString mActiveCard;

    enum ReadState {
        PhoneBookIdRead = 0x1,
        PhoneBookLimitsRead = 0x2,
        PhoneBookEntriesRead = 0x4,
        PhoneBookRead = PhoneBookIdRead | PhoneBookLimitsRead | PhoneBookEntriesRead
    };
    int readState;

    int SIMLabelLimit;
    int SIMNumberLimit;
    int SIMListStart;
    int SIMListEnd;

    bool readingSim;

    QPhoneBook *mPhoneBook;
    QSimInfo *mSimInfo;

    QList<QPhoneBookEntry> phoneData;
    ContactSqlIO *mAccess;
};
#endif // _QSIMCONTEXT_P_H_
