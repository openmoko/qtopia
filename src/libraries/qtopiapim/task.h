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
#ifndef QTOPIAPIM_TASK_H__
#define QTOPIAPIM_TASK_H__

#include <qtopia/stringutil.h>
#include <qtopia/pim/pimrecord.h>

#include <qvaluelist.h>
#include <qdatetime.h>

class PimTaskPrivate;
class TodoAccessPrivate;

class QTOPIAPIM_EXPORT PimTask : public PimRecord
{
    friend class TodoAccessPrivate;
public:
    enum TaskStatus {
	NotStarted = 0,
	InProgress,
	Completed,
	Waiting,
	Deferred
    };
    
    /*!
      \internal
    */
    enum TaskFields {
	HasDate = CommonFieldsEnd,
	Description,
	Priority,
	CompletedField,
	PercentCompleted,
	DueDateYear,
	DueDateMonth,
	DueDateDay,
	Status,
	StartedDate,
	CompletedDate,
	Notes,
	
	TaskFieldsEnd = 100
    };
    
    enum PriorityValue { VeryHigh=1, High, Normal, Low, VeryLow };

    PimTask();
    void fromMap( const QMap<int,QString> &);

    virtual ~PimTask();

    static void writeVCalendar( const QString &filename, const QValueList<PimTask> &tasks);
    static void writeVCalendar( const QString &filename, const PimTask &task);
    static QValueList<PimTask> readVCalendar( const QString &filename );

    void setPriority( PriorityValue priority ) { mPriority = priority; }
    PriorityValue priority() const { return mPriority; }

    void setDescription( const QString& description )
    { mDesc = description; }
    QString description() const { return mDesc; }

    void setDueDate( const QDate& date);
    void clearDueDate() { mDue = FALSE; }
    
    const QDate &dueDate() const { return mDueDate; }
    bool hasDueDate() const { return mDue; }
    
    const QDate &startedDate() const { return mStartedDate; }
    void setStartedDate(const QDate &date) { mStartedDate = date; }
    bool hasStartedDate() const;
    
    const QDate &completedDate() const { return mCompletedDate; }
    void setCompletedDate(const QDate &date) { mCompletedDate = date; }
    
    TaskStatus status() const; 
    void setStatus(TaskStatus s);
    
    bool isCompleted() const { return mCompleted; }
    void setCompleted( bool b ) { mCompleted = b; }

    uint percentCompleted() const;
    void setPercentCompleted( uint u );
    
    QString notes() const { return mNotes; }
    void setNotes(const QString &s) { mNotes = s; }

    bool match( const QRegExp &r ) const;

    QString toRichText() const;

    virtual void setFields(const QMap<int,QString> &);
    
    virtual void setField(int,const QString &);
    virtual QString field(int) const;
    virtual QMap<int, QString> fields() const;
    
    static const QMap<int, QCString> &keyToIdentifierMap();
    static const QMap<QCString,int> &identifierToKeyMap();
    static const QMap<int, QString> & trFieldsMap();
    // needed for Qtopia Desktop synchronization
    static const QMap<int,int> &uniquenessMap();

#ifndef QT_NO_DATASTREAM
    friend QTOPIAPIM_EXPORT QDataStream &operator>>( QDataStream &, PimTask & );
    friend QTOPIAPIM_EXPORT QDataStream &operator<<( QDataStream &, const PimTask & );
#endif

//protected:
    //virtual int endFieldMarker() const {return TaskFieldCount; }

private:
    static TaskStatus xmlToStatus(const QString &s);
    static QString statusToXml(TaskStatus);
    static void initMaps();
    
private:
    bool mDue;
    QDate mDueDate;
    bool mCompleted;
    PriorityValue mPriority;
    QString mDesc;
    
    TaskStatus mStatus;
    QDate mStartedDate, mCompletedDate;
    QString mNotes;
    uint mPercentCompleted;
    
    PimTaskPrivate *d;
};


#ifndef QT_NO_DATASTREAM
QTOPIAPIM_EXPORT QDataStream &operator>>( QDataStream &, PimTask & );
QTOPIAPIM_EXPORT QDataStream &operator<<( QDataStream &, const PimTask & );
#endif

#endif
