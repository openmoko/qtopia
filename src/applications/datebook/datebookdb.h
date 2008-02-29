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
