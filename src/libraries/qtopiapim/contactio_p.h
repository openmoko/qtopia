/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info\@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef CONTACTIO_PRIVATE_H
#define CONTACTIO_PRIVATE_H

#include <qvector.h>
#include <qvaluelist.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qobject.h>
#include <qtopia/pim/contact.h>

class ContactIO;
class PrContact : public PimContact {
public:
    PrContact() : PimContact() {}
    PrContact(const PrContact &t) : PimContact(t) {}

    void setUid(QUuid u) { p_setUid(u); }

    void insertField(int k, const QString &v) { insert(k,v); }
    const QMap<int, QString> &mapRef() const { return mMap; }
};


class ContactIteratorMachine : public QShared
{
public:
    virtual ~ContactIteratorMachine() {}

    virtual bool atFirst() const = 0;
    virtual bool atLast() const = 0;
    virtual const PrContact *toFirst() = 0;
    virtual const PrContact *toLast() = 0;

    virtual const PrContact *next() = 0;
    virtual const PrContact *prev() = 0;

    virtual const PrContact *current() const = 0;
};

class ContactIO : public QObject {

Q_OBJECT

public:
    enum AccessMode {
	ReadOnly,
	ReadWrite
    };

    ContactIO(AccessMode m) : amode(m) { }
    virtual ~ContactIO() {}

    AccessMode accessMode() const { return amode; }

    virtual ContactIteratorMachine *begin() const = 0;

signals:
    void contactsUpdated();

private:
    AccessMode amode;
};

#endif
