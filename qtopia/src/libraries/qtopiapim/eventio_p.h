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

#ifndef EVENTIO_PRIVATE_H
#define EVENTIO_PRIVATE_H

#include <qvector.h>
#include <qvaluelist.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qobject.h>
#include <qtopia/pim/event.h>

class EventIO;
class QTOPIAPIM_EXPORT PrEvent : public PimEvent {
public:
    PrEvent() : PimEvent() {}
    PrEvent(const PrEvent &t) : PimEvent(t) {}

    void setUid(QUuid u) { p_setUid(u); }

    uchar p_weekMask() const { return weekMask; }
    void p_setWeekMask(uchar v) { weekMask = v; }

    time_t startAsUTC() const { return PimEvent::startAsUTC(); }
    time_t endAsUTC() const { return PimEvent::endAsUTC(); }
    time_t repeatTillAsUTC() const { return PimEvent::repeatTillAsUTC(); }

    // set the timezone first before calling this, if timezone is to be set.
    void setStartAsUTC(time_t t) { PimEvent::setStartAsUTC(t); }
    void setEndAsUTC(time_t t) { PimEvent::setEndAsUTC(t); }
    void setRepeatTillAsUTC(time_t t) { PimEvent::setRepeatTillAsUTC(t); }

    const QValueList<QDate> &exceptions() const { return mExceptions; }
    const QValueList<QUuid> &childUids() const { return mChildren; }
    QUuid parentUid( ) const { return mParent; }

    void addException( const QDate &d ) { mExceptions.append(d); }
    void addChildUid( const QUuid &u ) { mChildren.append(u); }
    void removeChildUid( const QUuid &u ) { mChildren.remove(u); }
    void setParentUid( const QUuid &u ) { mParent = u; }
};


class QTOPIAPIM_EXPORT EventIteratorMachine : public QShared
{
public:
    virtual ~EventIteratorMachine() {}

    virtual bool atFirst() const = 0;
    virtual bool atLast() const = 0;
    virtual const PrEvent *toFirst() = 0;
    virtual const PrEvent *toLast() = 0;

    virtual const PrEvent *next() = 0;
    virtual const PrEvent *prev() = 0;

    virtual const PrEvent *current() const = 0;
};

class QTOPIAPIM_EXPORT EventIO : public QObject {

Q_OBJECT

public:
    enum AccessMode {
	ReadOnly,
	ReadWrite,
	WriteOnly
    };

    EventIO(AccessMode m) : amode(m) { }
    virtual ~EventIO() {}

    AccessMode accessMode() const { return amode; }

    virtual EventIteratorMachine *begin() const = 0;

    virtual PrEvent eventForId( const QUuid &, bool *ok ) const = 0;
signals:
    void eventsUpdated();

private:
    AccessMode amode;
};

#endif
