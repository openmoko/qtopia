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

#ifndef CONTACTIO_PRIVATE_H
#define CONTACTIO_PRIVATE_H

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

#include <QVector>
#include <QList>
#include <QMap>
#include <QRegExp>
#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qcontactmodel.h>

#include "qrecordio_p.h"

class QPimSource;
class QContactIO : public QRecordIO {

Q_OBJECT

public:

    explicit QContactIO(QObject *parent);
    virtual ~QContactIO();

    static QList<QContactModel::Field> labelKeys();
    static QStringList labelIdentifiers();

    static void setFormat(const QString &);
    static QString format();
    static QList<QContactModel::Field> formatFieldOrder();

    static int formatCount();
    static QList<QVariant> format(int);

    static QString formattedLabel(const QContact &);
    virtual QString formattedLabel(int row);

    virtual void setSortKey(QContactModel::Field k) = 0;
    virtual QContactModel::Field sortKey() const = 0;

    virtual QContact contact(const QUniqueId &) const = 0;
    virtual QContact contact(int row) const = 0;

    virtual QContact simpleContact(int row) const
    { return contact(row); }

    virtual QVariant contactField(int row, QContactModel::Field k) const;
    virtual bool setContactField(int row, QContactModel::Field k,  const QVariant &);

    virtual bool removeContact(int row) = 0;
    virtual bool removeContact(const QUniqueId & id) = 0;
    virtual bool removeContact(const QContact &) = 0;
    virtual bool removeContacts(const QList<int> &rows) = 0;
    virtual bool removeContacts(const QList<QUniqueId> &ids) = 0;

    virtual bool updateContact(const QContact &) = 0; // not done by row
    virtual QUniqueId addContact(const QContact &, const QPimSource &) = 0; // not done by row

    // override all or none.
    virtual void setFilter(const QString &, int) = 0;
    virtual void clearFilter() = 0;

    virtual QUniqueId matchEmailAddress(const QString &, int &) const;
#ifdef QTOPIA_PHONE
    virtual QUniqueId matchPhoneNumber(const QString &, int &) const;

    static QMap<QChar, QString> phoneButtonText();
    static QList<QContactModel::Field> labelSearchFields();
    static QList<QContactModel::Field> phoneNumberSearchFields();
#endif
signals:
    void labelFormatChanged();

public slots:
    virtual void checkAdded(const QUniqueId &);
    virtual void checkRemoved(const QUniqueId &);
    virtual void checkRemoved(const QList<QUniqueId> &);
    virtual void checkUpdated(const QUniqueId &);

protected:
    void notifyAdded(const QUniqueId &);
    void notifyUpdated(const QUniqueId &);
    void notifyRemoved(const QUniqueId &);
    void notifyRemoved(const QList<QUniqueId> &);

    virtual void ensureDataCurrent(bool force = false);

private slots:
    void pimMessage(const QString &message, const QByteArray &data);

private:
    void emitLabelFormatChanged();

    static void initFormat();

    static QList< QList<QVariant> > mFormat;
    static QList< QContactModel::Field > mFormatFieldOrder;
    static QList<QContactIO *> activeContacts;
#ifdef QTOPIA_PHONE
    static bool mPhoneButtonTextRead;
    static QMap<QChar, QString> mPhoneButtonText;
#endif
};

#endif
