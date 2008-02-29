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

class DateBookTablePrivate;

class EventXmlIO;

class DateBookTable : public QObject
{
    Q_OBJECT

public:
    DateBookTable(QObject *parent = 0, const char *name = 0);
    ~DateBookTable();

    QValueList<Occurrence> getOccurrences( const QDate &from,
	    const QDate &to ) const;

    QValueList<Occurrence> getNextAlarm( const QDateTime &from, int warn) const;
    Occurrence find(const QRegExp &, int, const QDate &, bool, bool *ok = 0) const;
    Occurrence find(const QUuid &u, const QDate &, bool *ok = 0) const;
    PimEvent find(const QUuid &u, bool *ok = 0) const;

    QUuid addEvent( const PimEvent &ev );
    void addException(const QDate &date, const PimEvent & );
    QUuid addException(const QDate &date, const PimEvent &, const PimEvent &);
    void removeExceptions(const PimEvent &ev);
    void removeEvent( const PimEvent &uid );

    void updateEvent( const PimEvent &event );
    void updateAlarm(const PimEvent &e);
    void updateAlarms();

    // This is needed for synchronization
    EventXmlIO &eventsIO() { return *dba; }

public slots:
    void externalAccess();
    void flush();
    void reload();

signals:
    void datebookUpdated();

private:
    DateBookTablePrivate *d;

    EventXmlIO *dba;
};

#endif
