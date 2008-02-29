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
#ifndef __TASK_H__
#define __TASK_H__

#include <qtopia/private/palmtoprecord.h>
#include <qtopia/stringutil.h>

#include <qvaluelist.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qmap.h>

class TaskPrivate;
class QTOPIA_EXPORT Task : public Qtopia::Record
{
public:
    Task();
    Task( const QMap<int, QString> &fromMap );
    ~Task();

    QMap<int, QString> toMap() const;

    static void writeVCalendar( const QString &filename, const QValueList<Task> &tasks);
    static void writeVCalendar( const QString &filename, const Task &task);
    static QValueList<Task> readVCalendar( const QString &filename );

    enum PriorityValue { VeryHigh=1, High, Normal, Low, VeryLow };

    void setPriority( int priority ) { mPriority = priority; }
    int priority() const { return mPriority; }

//     void setCategory( const QString& category )
//     { mCategory = category.stripWhiteSpace(); }
//     const QString &category() const { return mCategory; }

    void setDescription( const QString& description )
    { mDesc = Qtopia::simplifyMultiLineSpace(description); }
    const QString &description() const { return mDesc; }

    // Use THESE functions
//    void setDueDate( const QDate &date);
//    void clearDueDate();
   
    // Instead of these functions.
    void setDueDate( const QDate& date, bool hasDue ) { mDueDate = date; mDue = hasDue; }
    void setHasDueDate( bool b ) { mDue = b; }

    const QDate &dueDate() const { return mDueDate; }
    bool hasDueDate() const { return mDue; }
    
    void setCompleted( bool b ) { mCompleted = b; }
    bool isCompleted() const { return mCompleted; }

    void save( QString& buf ) const;
    bool match( const QRegExp &r ) const;

private:
#ifndef Q_OS_WIN32
    Qtopia::UidGen &uidGen() { return sUidGen; }
#else
	Qtopia::UidGen &uidGen();
#endif
    static Qtopia::UidGen sUidGen;

    bool mDue;
    QDate mDueDate;
    bool mCompleted;
    int mPriority;
    QString mDesc;
    TaskPrivate *d;
    // ADDITION
    int recordId;
    int recordInfo;
    //
};

// MUST be inline.  (forwards compatability).
//inline void Task::setDueDate( const QDate &date) { setDueDate(date, date.isValid()); }
//inline void Task::clearDueDate() { setHasDueDate( FALSE ); }
#endif
