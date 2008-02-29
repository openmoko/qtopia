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

#include <qtopia/qpeglobal.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qvaluelist.h>

// journal actions...
enum journal_action { ACTION_ADD, ACTION_REMOVE, ACTION_REPLACE };

class DateBookDBPrivate;
class QTOPIA_EXPORT DateBookDB
{
public:
    DateBookDB();
    ~DateBookDB();

    // very depreciated now!!!
    QValueList<Event> getEvents( const QDate &from, const QDate &to );
    QValueList<Event> getEvents( const QDateTime &start );

    // USE THESE!!!
    QValueList<EffectiveEvent> getEffectiveEvents( const QDate &from,
						   const QDate &to );
    QValueList<EffectiveEvent> getEffectiveEvents( const QDateTime &start );

    QValueList<Event> getRawRepeats() const;
    QValueList<Event> getNonRepeatingEvents( const QDate &from,
					     const QDate &to ) const;

    // Use these when dealing with adding removing events...
    void addEvent( const Event &ev, bool doalarm=TRUE );
    void removeEvent( const Event &ev );
    void editEvent( const Event &old, Event &ev );
    // add/remove event without journaling ( these ended up in public by accident, never
    // use them unless you know what you are doing...),
    // please put them in private if we ever can change the class...
    void addJFEvent( const Event &ev, bool doalarm=TRUE );
    void removeJFEvent( const Event &ev );

    bool save();
    void reload();
private:
    //find the real repeat...
    bool origRepeat( const Event &ev, Event &orig ) const;
    bool removeRepeat( const Event &ev );
    void init();
    void loadFile( const QString &strFile );
    // depreciated...
    void saveJournalEntry( const Event &ev, journal_action action );
    // new version, uncomment the "= -1" when we remove the above
    // function..
    bool saveJournalEntry( const Event &ev, journal_action action,
			   int key/* = -1*/, bool origHadRepeat = false );

    QValueList<Event> eventList; // non-repeating events...
    QValueList<Event> repeatEvents; // the repeating events...
    DateBookDBPrivate *d;
    QFile journalFile;

    int	recordIdMax;  // ADDITION
};

/* helper functions, also useful to other apps. */
QTOPIA_EXPORT bool nextOccurance( const Event &e, const QDate &from, QDateTime &next);
#endif
