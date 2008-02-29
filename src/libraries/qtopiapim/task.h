/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef __TASK_H__
#define __TASK_H__

#include <qtopia/stringutil.h>
#include <qtopia/pim/pimrecord.h>

#include <qvaluelist.h>
#include <qdatetime.h>

class PimTaskPrivate;
class TodoAccessPrivate;

class QTOPIA_EXPORT PimTask : public PimRecord
{
    friend class TodoAccessPrivate;
public:
    PimTask();
    virtual ~PimTask();

    static void writeVCalendar( const QString &filename, const QValueList<PimTask> &tasks);
    static void writeVCalendar( const QString &filename, const PimTask &task);
    static QValueList<PimTask> readVCalendar( const QString &filename );

    void setPriority( int priority ) { mPriority = priority; }
    int priority() const { return mPriority; }

    void setDescription( const QString& description )
    { mDesc = description; }
    const QString &description() const { return mDesc; }

    void setDueDate( const QDate& date) { mDueDate = date; mDue = TRUE; }
    void clearDueDate() { mDue = FALSE; }
    
    const QDate &dueDate() const { return mDueDate; }
    bool hasDueDate() const { return mDue; }
    
    void setCompleted( bool b ) { mCompleted = b; }
    bool isCompleted() const { return mCompleted; }

    bool match( const QRegExp &r ) const;

#ifndef QT_NO_DATASTREAM
    friend QTOPIA_EXPORT QDataStream &operator>>( QDataStream &, PimTask & );
    friend QTOPIA_EXPORT QDataStream &operator<<( QDataStream &, const PimTask & );
#endif

private:
    bool mDue;
    QDate mDueDate;
    bool mCompleted;
    int mPriority;
    QString mDesc;
    PimTaskPrivate *d;
};


#ifndef QT_NO_DATASTREAM
QTOPIA_EXPORT QDataStream &operator>>( QDataStream &, PimTask & );
QTOPIA_EXPORT QDataStream &operator<<( QDataStream &, const PimTask & );
#endif

#endif
