/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
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
class QTOPIAPIM_EXPORT PrContact : public PimContact {
public:
    PrContact() : PimContact() {}
    PrContact( const PimContact &t) : PimContact(t) { }
    PrContact(const PrContact &t) : PimContact(t) {}

    void setUid(QUuid u) { p_setUid(u); }

    void insertField(int k, const QString &v) { insert(k,v); }
    const QMap<int, QString> &mapRef() const { return mMap; }
};


class QTOPIAPIM_EXPORT ContactIteratorMachine : public QShared
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

class QTOPIAPIM_EXPORT ContactIO : public QObject {

Q_OBJECT

public:
    enum AccessMode {
	ReadOnly,
	ReadWrite,
	WriteOnly
    };

    ContactIO(AccessMode m) : amode(m) { }
    virtual ~ContactIO() {}

    AccessMode accessMode() const { return amode; }

    virtual ContactIteratorMachine *begin() const = 0;

    virtual PrContact contactForId( const QUuid &, bool *ok ) const = 0;

    virtual PrContact personal() const = 0;
    virtual void setAsPersonal(const QUuid &) = 0;

signals:
    void contactsUpdated();

private:
    AccessMode amode;
};

#endif
