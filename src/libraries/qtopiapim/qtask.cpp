/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtask.h"

#include "vobject_p.h"

#include <qtimestring.h>
#include <QRegExp>
#include <QString>
#include <QTextCodec>
#include <QApplication>
#include <QTextDocument>
#include <QDataStream>

#include <stdio.h>

class QTaskData : public QSharedData
{
public:
    bool mDue;
    QDate mDueDate;
    int mPriority;
    QString mDesc;

    int mStatus;
    QDate mStartedDate, mCompletedDate;
    QString mNotes;
    uint mPercentCompleted;

    QUniqueId mUid;
    QList<QString> mCategories;
    QMap<QString, QString> customMap;
};


/*!
  \class QTask
  \module qpepim
  \ingroup pim
  \brief The QTask class holds the data of a todo entry.

  This data includes the priority of the task, a description, an optional due
  date, and whether the task is completed or not.
*/



/*!
  Constructs a QTask as a copy of \a other.
*/
QTask::QTask(const QTask &other) : QPimRecord(other)
{
    d = other.d;
}

/*!
  Sets the task to be a copy of \a other.
*/
QTask &QTask::operator=(const QTask &other)
{
    d = other.d;
    return *this;
}

/*!
  Sets the priority of the task to \a priority.

  \sa priority()
*/
void QTask::setPriority( Priority priority ) { d->mPriority = priority; }

/*!
  Sets the priority of the task to \a priority.

  \sa priority()
*/
void QTask::setPriority( int priority ) {
    if (priority >= VeryHigh && priority <= VeryLow)
        setPriority((Priority)priority);
}

/*!
  Returns the priority of the task.

  \sa setPriority()
*/
QTask::Priority QTask::priority() const { return (Priority)d->mPriority; }

/*!
  Returns the notes for the task.

  \sa setNotes()
*/
QString QTask::notes() const { return d->mNotes; }

/*!
  Sets the notes of the task to \a s.

  \sa notes()
*/
void QTask::setNotes(const QString &s) { d->mNotes = s; }

/*!
  Sets the description of the task to \a description.

  \sa description()
 */
void QTask::setDescription( const QString& description )
{ d->mDesc = description; }

/*!
  Returns the description of the task.

  \sa setDescription()
 */
QString QTask::description() const { return d->mDesc; }

/*!
  Clears the due date of the task.

  \sa setDueDate()
*/
void QTask::clearDueDate() { d->mDue = false; }

/*!
  If \a b is true marks the task as completed.  Otherwise marks the task as
  uncompleted, and if necessary, the percentage complete will be adjusted to 99
  percent.

  \sa isCompleted()
*/
void QTask::setCompleted( bool b )
{
    if (b == isCompleted())
        return;

    d->mCompletedDate = b ? QDate::currentDate() : QDate();
    /* Might also need to set the percent completed to something less than 100 */
    if ( !b && d->mPercentCompleted == 100 )
        d->mPercentCompleted = 99;
}

/*!
  Returns true if the task is completed.  Otherwise returns false.

  \sa setCompleted()
*/
bool QTask::isCompleted() const { return status() == Completed; }

/*!
  Returns the due date of the task.

  \sa setDueDate(), clearDueDate()
 */
QDate QTask::dueDate() const { return d->mDueDate; }

/*!
  Returns true if there is a due date set for the task.  Otherwise returns
  false.

  \sa dueDate(), setDueDate(), clearDueDate()
*/
bool QTask::hasDueDate() const { return d->mDue; }

/*!
  Returns the date the task was started.  If the task has not yet been started, the returned
  date is undefined.

  \sa hasStartedDate(), setStartedDate()
*/
QDate QTask::startedDate() const { return d->mStartedDate; }

/*!
  Sets the tasks to have started on \a date.

  \sa hasStartedDate(), startedDate()
*/
void QTask::setStartedDate(const QDate &date) { d->mStartedDate = date; }

/*!
  Returns the date the task was completed.  If the task is not completed, the returned
  date is undefined.

  \sa isCompleted(), setCompletedDate()
*/
QDate QTask::completedDate() const { return d->mCompletedDate; }

/*!
  Sets the tasks completed date to \a date.

  \sa isCompleted(), completedDate()
*/
void QTask::setCompletedDate(const QDate &date) { d->mCompletedDate = date; }

/*!
  \overload
*/
QUniqueId &QTask::uidRef() { return d->mUid; }
/*!
  \overload
*/
const QUniqueId &QTask::uidRef() const { return d->mUid; }

/*!
  \overload
*/
QList<QString> &QTask::categoriesRef() { return d->mCategories; }
/*!
  \overload
*/
const QList<QString> &QTask::categoriesRef() const { return d->mCategories; }

/*!
  \overload
*/
QMap<QString, QString> &QTask::customFieldsRef() { return d->customMap; }
/*!
  \overload
*/
const QMap<QString, QString> &QTask::customFieldsRef() const { return d->customMap; }

/*!
  \enum QTask::Status

  These enums describe the current \l status() of the Task.

  The values are:

  \value NotStarted
  \value InProgress
  \value Completed
  \value Waiting
  \value Deferred
*/

/*!
  \enum QTask::Priority

  These enums describe the current \l priority() of the Task.

  The values are:

  \value VeryHigh
  \value High
  \value Normal
  \value Low
  \value VeryLow
*/

/*!
  Creates a new, empty task.
*/
QTask::QTask() : QPimRecord()
{
    d = new QTaskData();
    d->mDue = false;
    d->mDueDate = QDate::currentDate();
    d->mPriority = Normal;
    d->mStatus = NotStarted;
    d->mPercentCompleted = 0;
}

/*!
  Returns true if \a other is identical to the task. Otherwise return false.
*/
bool QTask::operator==(const QTask &other) const
{
    if (!(QPimRecord::operator==(other))) return false;
    if (d->mDue != other.d->mDue) return false;
    if (d->mPriority != other.d->mPriority) return false;
    if (d->mDesc != other.d->mDesc) return false;
    if (status() != other.status()) return false;
    if (d->mStartedDate != other.d->mStartedDate) return false;
    if (d->mCompletedDate != other.d->mCompletedDate) return false;
    if (d->mNotes != other.d->mNotes) return false;
    if (percentCompleted() != other.percentCompleted()) return false;
    return true;
}

/*!
  Returns false if \a other is identical to the task. Otherwise return true.
*/
bool QTask::operator!=(const QTask &other) const
{
    return !(*this == other);
}

/*!
  Destroys the task
*/
QTask::~QTask()
{
}

/*!
  Returns the \l Status of the task.

  \sa setStatus()
*/
QTask::Status QTask::status() const
{
    if ( !d->mCompletedDate.isNull())
        return Completed;
    if ( d->mStartedDate.isNull())
        return NotStarted;
    // and the revers needs to hold true as well, only send completed not started for the above state.
    if (d->mStatus == Completed || d->mStatus == NotStarted)
        return InProgress;
    return (Status)d->mStatus;
}

/*!
  Sets the \l Status of the task to \a s.

  \sa status()
*/
void QTask::setStatus(Status s)
{
    d->mStatus = s;
    // check date fields.
    if (d->mStartedDate.isNull()) {
        if (s != NotStarted)
            d->mStartedDate = QDate::currentDate();
    } else {
        if (s == NotStarted)
            d->mStartedDate = QDate();
    }
    if (d->mCompletedDate.isNull()) {
        if (s == Completed)
            d->mCompletedDate = QDate::currentDate();
    } else {
        if (s != Completed)
            d->mCompletedDate = QDate();
    }
    if (s != Completed && d->mPercentCompleted == 100)
        d->mPercentCompleted = 99;
}

/*!
  Sets the \l Status of the task to \a s.

  \sa status()
*/
void QTask::setStatus(int s)
{
    if (s >= NotStarted && s <= Deferred)
        setStatus((Status)s);
}

/*!
  Returns the translated text for the the task status \a s.
*/
QString QTask::statusToText(Status s)
{
    if (s < NotStarted || s > Deferred)
        return QString();

    static const char *const status_strings[] = {
        QT_TRANSLATE_NOOP( "QTask", "Not Started" ),
        QT_TRANSLATE_NOOP( "QTask", "In Progress" ),
        QT_TRANSLATE_NOOP( "QTask", "Completed" ),
        QT_TRANSLATE_NOOP( "QTask", "Waiting" ),
        QT_TRANSLATE_NOOP( "QTask", "Deferred" ),
    };

    if (qApp)
        return qApp->translate("QTask", status_strings[s]);
    else
        return status_strings[s];
}

/*!
  Sets the due date of the task to \a date.

  \sa clearDueDate(), dueDate()
*/
void QTask::setDueDate( const QDate &date )
{
    d->mDueDate = date;
    d->mDue = !(date.isNull());
}

/*!
  Returns an int indicating the percent completed of the task.
  For completed tasks, this function will always return 100.

  \sa setPercentCompleted(), status()
*/
uint QTask::percentCompleted() const
{
    QTask::Status s = status();
    if (s == NotStarted)
        return 0;
    if (s == Completed)
        return 100;
    return d->mPercentCompleted;
}

/*!
  Sets the tasks percent completed field to \a percent.
  if \a percent is greater than 99 this function will also
  set the status to Completed.

  \sa percentCompleted(), status()
*/
void QTask::setPercentCompleted( uint percent )
{
    if (percent > 99) {
        setStatus(Completed);
        d->mPercentCompleted = 100;
    } else if (percent == 0) {
        setStatus(NotStarted);
        d->mPercentCompleted = 0;
    } else {
        QTask::Status s = status();
        if (s == NotStarted || Completed)
            setStatus(InProgress);
        d->mPercentCompleted = percent;
    }
}

/*!
  Returns true if the task has a started date.

  \sa startedDate(), setStartedDate()
*/

bool QTask::hasStartedDate() const
{
    return !d->mStartedDate.isNull();
}

static QString statusToTrString(QTask::Status s)
{
    switch( s ) {
        case QTask::NotStarted: return qApp->translate("QtopiaPim", "Not yet started"); break;
        case QTask::InProgress: return qApp->translate("QtopiaPim", "In progress"); break;
        case QTask::Waiting: return qApp->translate("QtopiaPim", "Waiting"); break;
        case QTask::Deferred: return qApp->translate("QtopiaPim", "Deferred"); break;
        default: return qApp->translate("QtopiaPim", "Completed"); break;
    }
}

/*!
  Returns true if the part of task matches \a r. Otherwise returns false.
*/
bool QTask::match ( const QRegExp &r ) const
{
    // match on priority, description on due date...
    bool match = false;
    if ( QString::number( d->mPriority ).contains( r ) )
        match = true;
    else if ( d->mDue && d->mDueDate.toString().contains( r ) )
        match = true;
    else if ( d->mDesc.contains( r ) )
        match = true;
    else if ( d->mStartedDate.isValid() && d->mStartedDate.toString().contains( r ) )
        match = true;
    else if ( d->mCompletedDate.isValid() && d->mCompletedDate.toString().contains( r ) )
        match = true;
    else if ( status() != NotStarted && status() != Completed &&
            QString::number(percentCompleted()).contains( r ) )
        match = true;
    else if ( d->mNotes.contains( r ) )
        match = true;
    else if ( statusToTrString( status() ).contains( r ) )
        match = true;

    return match;
}

// In pimrecord.cpp
void qpe_startVObjectInput();
bool qpe_vobjectCompatibility(const char* misfeature);
void qpe_endVObjectInput();
void qpe_startVObjectOutput();
void qpe_endVObjectOutput(VObject *,const char* type,const QPimRecord*);
void qpe_setVObjectProperty(const QString&, const QString&, const char* type, QPimRecord*);
VObject *qpe_safeAddPropValue( VObject *o, const char *prop, const QString &value );
static inline VObject *safeAddPropValue( VObject *o, const char *prop, const QString &value )
{ return qpe_safeAddPropValue(o,prop,value); }
VObject *qpe_safeAddProp( VObject *o, const char *prop);
static inline VObject *safeAddProp( VObject *o, const char *prop)
{ return qpe_safeAddProp(o,prop); }

static VObject *createVObject( const QTask &t )
{
    qpe_startVObjectOutput();

    VObject *vcal = newVObject( VCCalProp );
    safeAddPropValue( vcal, VCVersionProp, "1.0" );
    VObject *task = safeAddProp( vcal, VCTodoProp );

    if ( t.hasDueDate() )
        safeAddPropValue( task, VCDueProp,
                t.dueDate().toString(Qt::ISODate) );
    if ( t.isCompleted() ) {
        // if we say its completed, then we have a completed date.
        safeAddPropValue( task, VCStatusProp, "COMPLETED" );
        safeAddPropValue( task, VCCompletedProp,
                t.completedDate().toString(Qt::ISODate) );
    }
    safeAddPropValue( task, VCPriorityProp, QString::number( t.priority() ) );

    // status *2 (enum && %)
    // ^^ We don't match VCStatusProp and vCal doesn't support ::percent.

    safeAddPropValue( task, "X-Qtopia-STATUS", QString::number( t.status() ));
    safeAddPropValue( task, "X-Qtopia-PERCOMP",
            QString::number( t.percentCompleted() ) );

    if (t.hasStartedDate())
    {
        // ok, need to set this one too.
        safeAddPropValue( task, "X-Qtopia-STARTED",
                t.startedDate().toString(Qt::ISODate));
    }

    // vCal spec: VCSummaryProp is required
    // Palm m100:     No (violates spec)
    // Ericsson T39m: Yes
    if ( qpe_vobjectCompatibility("Palm-Task-DN") ) {
        safeAddPropValue( task, VCSummaryProp, t.description() );
        safeAddPropValue( task, VCDescriptionProp, t.description() );
        safeAddPropValue( task, VCAttachProp, t.notes() );
    } else {
        safeAddPropValue( task, VCSummaryProp, t.description() );
        safeAddPropValue( task, VCDescriptionProp, t.notes() );
    }

    qpe_endVObjectOutput(task,"Todo List",&t); // No tr

    return vcal;
}


static QTask parseVObject( VObject *obj )
{
    QTask t;

    VObjectIterator it;
    initPropIterator( &it, obj );
    QString summary, description, attach; // vCal properties, not Qtopias
    while( moreIteration( &it ) ) {
        VObject *o = nextVObject( &it );
        QString name = vObjectName( o );

        // check this key/value for a CHARSET field.
        VObjectIterator tnit;
        initPropIterator( &tnit, o );
        QTextCodec *tc = 0;
        while( moreIteration( &tnit ) ) {
            VObject *otc = nextVObject( &tnit );
            if ( qstrcmp(vObjectName(otc), VCCharSetProp ) == 0) {
                tc = QTextCodec::codecForName(vObjectStringZValue(otc));
                break;
            }
        }
        QString value;
        if (tc)
            value = tc->toUnicode( vObjectStringZValue( o ) );
        else
            value = vObjectStringZValue( o );

        if ( name == VCDueProp ) {
            t.setDueDate( QDate::fromString( value, Qt::ISODate ) );
        }
        else if ( name == VCSummaryProp ) {
            summary = value;
        }
        else if ( name == VCDescriptionProp ) {
            description = value;
        }
        else if (name == VCAttachProp ) {
            attach = value;
        }
        else if ( name == VCStatusProp ) {
            if ( value == "COMPLETED" )
                t.setCompleted( true );
        }
        else if ( name == VCCompletedProp ) {
            t.setCompletedDate( QDate::fromString( value ) );
        }
        else if ( name == VCPriorityProp ) {
            t.setPriority( (QTask::Priority) value.toInt() );
        }
        else if (name == "X-Qtopia-STATUS" ) {
            t.setStatus( (QTask::Status) value.toInt() );
        }
        else if (name == "X-Qtopia-PERCOMP" ) {
            t.setPercentCompleted( value.toInt() );
        }
        else if (name == "X-Qtopia-STARTED" ) {
            t.setStartedDate( QDate::fromString( value, Qt::ISODate ) );
        } else {
            qpe_setVObjectProperty(name,value,"Todo List",&t); // No tr
        }
    }

    // Find best mapping from (Summary,Description,Attach) to our (Description,Notes)
    // Similar code in event.cpp
    if ( !summary.isEmpty() && !description.isEmpty() && summary != description ) {
        t.setDescription( summary );
        t.setNotes( description );
        // all 3 - drop attach
    } else if ( !summary.isEmpty() ) {
        t.setDescription( summary );
        t.setNotes( attach );
    } else {
        t.setDescription( description );
        t.setNotes( attach );
    }

    return t;
}

/*!
   Write the list of \a tasks as vCalendar objects to the file
   specified by \a filename.

   \sa readVCalendar()
*/
void QTask::writeVCalendar( const QString &filename, const QList<QTask> &tasks)
{
    FILE *f = fopen(filename.toLocal8Bit(),"w");
    if ( !f ) {
        qWarning("Unable to open vcard write");
        return;
    }

    QList<QTask>::ConstIterator it;
    for( it = tasks.begin(); it != tasks.end(); ++it ) {
        VObject *obj = createVObject( *it );
        writeVObject(f, obj );
        cleanVObject( obj );
    }

    cleanStrTbl();
    fclose(f);
}

/*!
   Write the \a task as a vCalendar to the file specified by \a filename.

   \sa readVCalendar()
*/
void QTask::writeVCalendar( const QString &filename, const QTask &task)
{
    FILE *f = fopen(filename.toLocal8Bit(),"w");
    if ( !f ) {
        qWarning("Unable to open vcard write");
        return;
    }

    VObject *obj = createVObject( task );
    writeVObject(f, obj );
    cleanVObject( obj );

    cleanStrTbl();
    fclose(f);
}

/*!
  Reads the file specified by \a filename as a list of vCalendar objects
  and returns the list of near equivalent tasks.

  \sa writeVCalendar()
*/
QList<QTask> QTask::readVCalendar( const QString &filename )
{
    VObject *obj = Parse_MIME_FromFileName( (const char *)filename.toUtf8() );

    QList<QTask> tasks;

    qpe_startVObjectInput();
    while ( obj ) {
        QString name = vObjectName( obj );
        if ( name == VCCalProp ) {
            VObjectIterator nit;
            initPropIterator( &nit, obj );
            while( moreIteration( &nit ) ) {
                VObject *o = nextVObject( &nit );
                QString name = vObjectName( o );
                if ( name == VCTodoProp )
                    tasks.append( parseVObject( o ) );
            }
        } else if ( name == VCTodoProp ) {
            // shouldn't happen, but just to be sure
            tasks.append( parseVObject( obj ) );
        }
        VObject *t = obj;
        obj = nextVObjectInList(obj);
        cleanVObject( t );
    }
    qpe_endVObjectInput();

    return tasks;
}

/*!
  Returns a rich text formatted QString of the QTask.
*/
QString QTask::toRichText() const
{
    QString text;

    text = "<center><b>" + Qt::escape(d->mDesc) + "</b></center><br>"; // No tr
    if ( d->mDue )
        text += "<b>" + qApp->translate("QtopiaPim", "Due:") + "</b> " +
            QTimeString::localYMD( d->mDueDate, QTimeString::Long ) + "<br>";
    if ( !d->mStartedDate.isNull() && status() != NotStarted)
        text += "<b>" + qApp->translate("QtopiaPim", "Started:") + "</b> " +
            QTimeString::localYMD( d->mStartedDate, QTimeString::Long ) +  "<br>";
    if ( !d->mCompletedDate.isNull() && isCompleted() )
        text += "<b>" + qApp->translate("QtopiaPim", "Completed:") + "</b> " +
            QTimeString::localYMD( d->mCompletedDate, QTimeString::Long ) + "<br>";

    //if ( !isCompleted() ) { //We remember old status and treat completed separately
    // such remembering is already done by, status();

    QString statusString = statusToTrString( status() );
    text += "<b>" + qApp->translate("QtopiaPim", "Status:") + "</b> " + statusString + "<br>";
    text +="<b>" + qApp->translate("QtopiaPim", "Priority:") + "</b> " + QString::number( d->mPriority ) + "<br>";

    if ( (status() != NotStarted && !isCompleted() ) )
        text += "<b>" + qApp->translate("QtopiaPim", "Completed:") + "</b> " + QString::number(percentCompleted()) + " " +
            qApp->translate("QtopiaPim", "percent", "Word or symbol after numbers for percentage") + "<br>";

    if ( !d->mNotes.isEmpty() )
        text += "<br> <b> " + qApp->translate("QtopiaPim", "Notes:") + "</b> "
            + d->mNotes;

    return text;
}


/*!
    \internal
    \fn void QTask::deserialize(Stream &s)
*/
template <typename Stream> void QTask::deserialize(Stream &s)
{
    s >> d->mUid;
    s >> d->mCategories;
    s >> d->customMap;
    quint8 val;
    s >> val;
    d->mDue = val == 0 ? false : true;
    s >> d->mDueDate;
    s >> val; // stream left to maintain compat, value ignored.
    //c.d->mCompleted = val == 0 ? false : true;
    qint32 p;
    s >> p;
    d->mPriority = (QTask::Priority) p;
    s >> d->mDesc;
    s >> d->mStartedDate;
    s >> d->mCompletedDate;

    qint32 i;
    s >> i;
    d->mStatus = (QTask::Status)i;

    s >> d->mNotes;
}

/*!
    \internal
    \fn void QTask::serialize(Stream &s) const
*/
template <typename Stream> void QTask::serialize(Stream &s) const
{
    s << d->mUid;
    s << d->mCategories;
    s << d->customMap;
    s << (d->mDue ? (uchar)1 : (uchar)0);
    s << d->mDueDate;
    s << (isCompleted() ? (uchar)1 : (uchar)0);
    s << d->mPriority;
    s << d->mDesc;
    s << d->mStartedDate;
    s << d->mCompletedDate;
    int i = (int) d->mStatus;
    s << i;
    s << d->mNotes;
}

Q_IMPLEMENT_USER_METATYPE(QTask)
