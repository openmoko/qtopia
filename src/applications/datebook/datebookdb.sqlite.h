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
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DATEBOOKDB_H
#define DATEBOOKDB_H

#include <qdatetime.h>
#include <qfile.h>
#include <qvaluelist.h>
#include <qobject.h>
#include <qtopia/pim/event.h>
#include <sqlite.h>

class DateBookTablePrivate;
class DatabaseTable;
class DatabaseIndex;

class DateBookTable;

class PimEventIndex;
class PimEventTable;

class Occurance
{
public:
    Occurance() {}
    Occurance(const Occurance &other) 
    {
	eventCache = other.eventCache;
	mStart = other.mStart;
	//set = other.set;
	//index = other.index;
    }

    Occurance& operator=(const Occurance &other)
    {
	eventCache = other.eventCache;
	mStart = other.mStart;
	//set = other.set;
	//index = other.index;

	return *this;
    }

    ~Occurance() {}

    QDate date() const { return mStart; }
    QDate endDate() const;
    QDateTime start() const;
    QDateTime end() const;

    PimEvent event() const;
    //PimEvent eventDirect() const;
private:
    friend class DateBookTable;
    //Occurance(const QDate &start, PimEventIndex *e, int i);
    Occurance(const QDate &start, const PimEvent &);

    mutable PimEvent eventCache;
    QDate mStart;
    //PimEventIndex *set;
    //int index;
};

class DateBookTable : public QObject
{
    Q_OBJECT

public:
    DateBookTable(QObject *parent = 0, const char *name = 0);
    ~DateBookTable();

    QValueList<Occurance> getOccurances( const QDate &from,
	    const QDate &to ) const;

    Occurance getNextAlarm( const QDateTime &from, bool *ok = 0) const;

    void addEvent( const PimEvent &ev );
    void removeEvent( const PimEvent &uid );

    void replaceEvent( const PimEvent &oldevent, const PimEvent &newevent );

signals:
    void eventAdded( const PimEvent & );
    void eventRemoved( const PimEvent & );

private:
    DateBookTablePrivate *d;

    sqlite *acc;
    PimEventTable *store;

    PimEventIndex *byEnd;
    PimEventIndex *byStart;
    PimEventIndex *byUid;
};

#endif
