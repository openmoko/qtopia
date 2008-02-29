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
#ifndef _QSIMCONTEXT_P_H_
#define _QSIMCONTEXT_P_H_

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopiasql.h>
#include <qpimsource.h>
#include <qcontact.h>
#include <qphonebook.h>
#include <qcontactmodel.h>

class ContactSqlIO;
class QSimInfo;
class QContactSimSyncer;
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
    QPimSource serviceNumbersSource() const;

    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> visibleSources() const;
    QSet<QPimSource> sources() const;
    QUuid id() const;

    using QContactContext::exists;
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
    void simIdentityUpdated(const QString &, const QDateTime &);

private slots:
    void notifyUpdate(int context);
    void readSimIdentity();

private:
    QString card(const QUniqueId &) const;
    int cardIndex(const QUniqueId &) const;
    QUniqueId findLabel(const QString &) const;

    int nextFreeIndex() const;

    bool isSIMContactCompatible(const QContact &c) const;

    static QString typeToSIMExtension(QContactModel::Field type);
    static QContactModel::Field SIMExtensionToType(QString &label);
    QString createSIMLabel(const QContact &c);

    ContactSqlIO *mAccess;
    QContactSimSyncer *mSync;
    QContactSimSyncer *mServiceNumbers;
    QPhoneBook *mPhoneBook;
    QSimInfo *mSimInfo;
};
#endif // _QSIMCONTEXT_P_H_
