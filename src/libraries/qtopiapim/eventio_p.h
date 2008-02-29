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

#ifndef EVENTIO_PRIVATE_H
#define EVENTIO_PRIVATE_H

#include <qvector.h>
#include <qvaluelist.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qobject.h>
#include <qtopia/pim/event.h>

class EventIO;
class PrEvent : public PimEvent {
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
};


class EventIteratorMachine : public QShared
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

class EventIO : public QObject {

Q_OBJECT

public:
    enum AccessMode {
	ReadOnly,
	ReadWrite
    };

    EventIO(AccessMode m) : amode(m) { }
    virtual ~EventIO() {}

    AccessMode accessMode() const { return amode; }

    virtual EventIteratorMachine *begin() const = 0;

signals:
    void eventsUpdated();

private:
    AccessMode amode;
};

#endif
