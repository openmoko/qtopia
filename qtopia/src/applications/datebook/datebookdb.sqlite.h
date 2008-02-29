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
