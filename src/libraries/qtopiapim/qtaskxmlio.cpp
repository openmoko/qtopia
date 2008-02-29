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

#include <qfile.h>
#include <qsettings.h>
#include <qtopianamespace.h>
#include <qfileinfo.h>
#include <qtopiaipcenvelope.h>
#include <qapplication.h>
#include <qregexp.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include "qtaskxmlio_p.h"
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif


static const XmlIOField  todolistentries[] = {
    { "HasDate", QTaskXmlIO::HasDate },
    { "Description", QTaskXmlIO::Description },
    { "Priority", QTaskXmlIO::Priority },
    { "Completed", QTaskXmlIO::CompletedField },
    { "PercentCompleted", QTaskXmlIO::PercentCompleted },
    { "DateYear", QTaskXmlIO::DueDateYear },
    { "DateMonth", QTaskXmlIO::DueDateMonth },
    { "DateDay", QTaskXmlIO::DueDateDay },
    { "Status", QTaskXmlIO::Status },
    { "StartedDate", QTaskXmlIO::StartedDate },
    { "CompletedDate", QTaskXmlIO::CompletedDate },
    { "Notes", QTaskXmlIO::Notes },
    { 0, 0 }
};

static const XmlIOField  todolistentries_opie[] = {
    { "HasDate", QTaskXmlIO::HasDate },
    { "Summary", QTaskXmlIO::Description },
    { "Priority", QTaskXmlIO::Priority },
    { "Completed", QTaskXmlIO::CompletedField },
    { "PercentCompleted", QTaskXmlIO::PercentCompleted },
    { "DateYear", QTaskXmlIO::DueDateMonth },
    { "DateDay", QTaskXmlIO::DueDateDay },
    { "Status", QTaskXmlIO::Status },
    { "StartedDate", QTaskXmlIO::StartedDate },
    { "CompletedDate", QTaskXmlIO::CompletedDate },
    { "Description", QTaskXmlIO::Notes },
    { 0, 0 }
};

class QTaskCompare
{
public:
    QTaskCompare() : mKey(QTaskModel::Invalid) {}

    bool operator()(const QTask *, const QTask *) const;
    bool compareTaskField(QTaskModel::Field, const QTask *, const QTask *) const;

    QTaskModel::Field mKey;
};

bool QTaskCompare::operator()(const QTask *t1, const QTask *t2) const
{
    int result = 0;
    if ( mKey != QTaskModel::Invalid )
        result = compareTaskField( mKey, t1, t2);
    if ( !result )
        result = compareTaskField( QTaskModel::Completed, t1, t2);
    if ( !result )
        result = compareTaskField( QTaskModel::Priority, t1, t2);
    if ( !result )
        result = compareTaskField( QTaskModel::DueDate, t1, t2);
    if ( !result )
        result = compareTaskField( QTaskModel::Description, t1, t2);

    return (result < 0);
}

bool QTaskCompare::compareTaskField(QTaskModel::Field key, const QTask *t1, const QTask *t2) const
{
    switch (key) {
        case QTaskModel::Completed:
            {
                int c = 0;
                if (t1->isCompleted())
                    c++;
                if (t2->isCompleted())
                    c--;

                return c;
            }
        case QTaskModel::Priority: return (t1->priority() - t2->priority() );
        case QTaskModel::DueDate:
            if (t1->hasDueDate() && t2->hasDueDate())
                return ( t2->dueDate().daysTo(t1->dueDate()) );
            else if (t1->hasDueDate())
                return -1;
            else if (t2->hasDueDate())
                return 1;

            return 0;
        case QTaskModel::Description: return QString::compare(t1->description().toLower(), t2->description().toLower());
        case QTaskModel::Invalid:
        default: return 0;
    }
}

QTaskXmlIO::QTaskXmlIO(AccessMode m,
                     const QString &file,
                     const QString &journal ) :
    QTaskIO(0, m), QPimXmlIO(),
    cFilter(QCategoryFilter::All), cCompFilter(false), cKey(QTaskModel::Invalid), needsSave(false)
{
    if ( file != QString() )
        setDataFilename( file );
    else setDataFilename( Qtopia::applicationFileName( "todolist", "todolist.xml" ) );
    if ( journal != QString() )
        setJournalFilename( journal );
    else setJournalFilename( Qtopia::applicationFileName( "todolist", "todolist.journal" ) );

    loadData();

    if (m == ReadOnly) {
        QtopiaChannel *channel = new QtopiaChannel( "QPE/PIM",  this );

        connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
                this, SLOT(pimMessage(const QString&,const QByteArray&)) );
    }
}

void QTaskXmlIO::pimMessage(const QString &message, const QByteArray &data)
{
    QDataStream ds(data);
    if (message == "addedTask(int,QTask)") {
        int pid;
        QTask task;
        ds >> pid;
        ds >> task;
        if (pid != getpid()) {
            internalAddRecord(new QTask(task));
        }
    } else if (message == "removedTask(int,QTask)") {
        int pid;
        QTask task;
        ds >> pid;
        ds >> task;
        if (pid != getpid()) {
            internalRemoveRecord(new QTask(task));
        }
    } else if (message == "updatedTask(int,QTask)") {
        int pid;
        QTask task;
        ds >> pid;
        ds >> task;
        if (pid != getpid()) {
            internalUpdateRecord(new QTask(task));
        }
    } else if (message == "reloadTasks()") {
        ensureDataCurrent();
    } else if ( message == "reload(int)" ) {
        int force;
        ds >> force;
        ensureDataCurrent(force);
    }
}

QTaskXmlIO::~QTaskXmlIO()
{
    if (accessMode() != ReadOnly )
        saveData();
}

bool QTaskXmlIO::internalAddRecord(QPimRecord *r)
{
    QTask *todo = (QTask *)r;

    m_Tasks.append( todo );
    if (select(*todo))
        m_Filtered.append(todo);

    return true;
}

bool QTaskXmlIO::internalRemoveRecord(QPimRecord *r)
{
    QTask *todo = (QTask *)r;

    QMutableListIterator<QTask*> it(m_Tasks);
    while(it.hasNext()) {
        QTask *current = it.next();
        if (current->uid() == todo->uid()) {
            if ( select( *current ) ) {
                m_Filtered.removeAll(current);
            }
            it.remove();
            delete r;
            return true;
        }
    }
    delete r;
    return false;
}

bool QTaskXmlIO::internalUpdateRecord(QPimRecord *r)
{
    QTask *todo = (QTask *)r;
    QMutableListIterator<QTask*> it(m_Tasks);
    while(it.hasNext()) {
        QTask *current = it.next();
        if (current->uid() == todo->uid()) {

            if ( select(*current) )
                m_Filtered.removeAll(current);
            if ( *current != *todo )
                *current = *todo;
            if (select(*current))
                m_Filtered.append(current);

            delete r;
            return true;
        }
    }
    delete r;
    return false;
}

/**
 * Returns the full task list.  This is guaranteed
 * to be current against what is stored by other apps.
 */
const QList<QTask*>& QTaskXmlIO::tasks() const {
  //ensureDataCurrent();
  return m_Tasks;
}

const QList<QTask*>& QTaskXmlIO::sortedTasks() const {
    //ensureDataCurrent();
    return m_Filtered;
}

void QTaskXmlIO::clear()
{
    cFilter = QCategoryFilter(QCategoryFilter::All);
    m_Filtered.clear();
    m_Tasks.clear();
    needsSave = true;
}

void QTaskXmlIO::sort()
{
    QTaskCompare c;
    c.mKey = cKey;
    qSort(m_Filtered.begin(), m_Filtered.end(), c);
}

/**
 * Saves the current task data.  Returns true if
 * successful.
 */
bool QTaskXmlIO::saveData(bool force)
{
    if ( force || !QFile::exists( dataFilename() ) || QFile::exists( journalFilename() ) )
        needsSave = true;

    if (!needsSave)
        return true;
    if (accessMode() != ReadOnly) {
        if (QPimXmlIO::saveData((QList<QPimRecord*> &)m_Tasks)) {
            needsSave = false;
            return true;
        }
    }
    return false;
}

bool QTaskXmlIO::loadData()
{
    if (QPimXmlIO::loadData()) {
        sort();
        return true;
    }
    return false;
}

QTask::Status xmlToStatus(const QString &s)
{
    if ( s == "InProgress" ) // No tr
        return QTask::InProgress;
    else if ( s == "Completed" ) // No tr
        return QTask::Completed;
    else if ( s == "Waiting" ) // No tr
        return QTask::Waiting;
    else if ( s == "Deferred" ) // No tr
        return QTask::Deferred;
    else
        return QTask::NotStarted;
}

QString statusToXml(QTask::Status status)
{
    switch( status ) {
        default: return "NotStarted"; // No tr
        case QTask::InProgress: return "InProgress"; // No tr
        case QTask::Completed: return "Completed"; // No tr
        case QTask::Waiting: return "Waiting"; // No tr
        case QTask::Deferred: return "Deferred"; // No tr
    }
}

void QTaskXmlIO::setFields(QPimRecord *r, const QMap<QString,QString> &recMap) const
{
    QTask *rec = (QTask *)r;
    QMap<QString,QString> m = recMap;

    bool tDue;
    int tDueDateDay = -1;
    int tDueDateMonth = -1;
    int tDueDateYear = -1;

    QMap<QString, QString>::Iterator it = m.begin();
    while (it != m.end()) {
        QString a = it.key();
        QString s = it.value();
        const XmlIOField *t = todolistentries;
        int key = -1;
        while (t->text) {
            if (a == t->text) {
                key = t->fieldId;
                break;
            }
            ++t;
        }

        switch( key ) {
            case HasDate:
                if ( s == "1" ) tDue = true;
                break;
            case Description:
                rec->setDescription( s );
                break;
            case Priority:
                rec->setPriority( (QTask::Priority) (s).toInt() );
                break;
            case CompletedField:
                rec->setCompleted( s == "1" );
                break;
            case PercentCompleted:
                rec->setPercentCompleted( s.toInt() );
                break;
            case DueDateYear:
                tDueDateYear = s.toInt();
                break;
            case DueDateMonth:
                tDueDateMonth = s.toInt();
                break;
            case DueDateDay:
                tDueDateDay = s.toInt();
                break;
            case Status:
                rec->setStatus( xmlToStatus(s) );
                break;
            case StartedDate:
                rec->setStartedDate( QPimXmlIO::xmlToDate( s ) );
                break;
            case CompletedDate:
                rec->setCompletedDate( QPimXmlIO::xmlToDate(s) );
                break;
            case Notes:
                rec->setNotes( s );
                break;
            default:
                break;
        }
        ++it;
    }
    if ( tDueDateYear != -1 && tDueDateMonth != -1 && tDueDateDay != -1 )
        rec->setDueDate(QDate(tDueDateYear, tDueDateMonth, tDueDateDay));
    else
        rec->clearDueDate();
    {
        const XmlIOField *t = todolistentries;
        while (t->text) {
            m.remove(t->text);
            ++t;
        }
    }

    // pass remaining fields to parent.
    QPimXmlIO::setFields(rec, m);
}

QMap<QString, QString> QTaskXmlIO::fields(const QPimRecord *r) const
{
    QTask *rec = (QTask *)r;
    QMap<QString,QString> m = QPimXmlIO::fields(r);

    const XmlIOField *t = todolistentries;
    while (t->text) {
        int key = t->fieldId;
        switch( key ) {
            case HasDate:
                if (rec->hasDueDate())
                    m.insert(t->text, "1");
                break;
            case Description:
                if (!rec->description().isEmpty())
                    m.insert(t->text, rec->description());
                break;
            case Priority:
                m.insert(t->text, QString::number(rec->priority()));
                break;
            case CompletedField:
                if (rec->isCompleted())
                    m.insert(t->text, "1"); // no tr;
                break;
            case PercentCompleted:
                m.insert(t->text, QString::number(rec->percentCompleted()));
                break;
            case DueDateYear:
                if (rec->hasDueDate())
                    m.insert(t->text, QString::number(rec->dueDate().year()));
                break;
            case DueDateMonth:
                if (rec->hasDueDate())
                    m.insert(t->text, QString::number(rec->dueDate().month()));
                break;
            case DueDateDay:
                if (rec->hasDueDate())
                    m.insert(t->text, QString::number(rec->dueDate().day()));
                break;
            case Status:
                m.insert(t->text, statusToXml(rec->status()));
                break;
            case StartedDate:
                if (rec->hasStartedDate())
                    m.insert(t->text, QPimXmlIO::dateToXml(rec->startedDate()));
                break;
            case CompletedDate:
                if (rec->isCompleted())
                    m.insert(t->text, QPimXmlIO::dateToXml(rec->completedDate())) ;
                break;
            case Notes:
                if (!rec->notes().isEmpty())
                    m.insert(t->text, rec->notes());
                break;
            default:
                break;
        }
        ++t;
    }
    return m;
}

QUniqueId QTaskXmlIO::addTask(const QTask &task)
{
    QUniqueId u;
    if (accessMode() == ReadOnly)
        return u;

    QTask *tsk = new QTask((const QTask &)task);

    //if ( assignUid || tsk->uid().isNull())
    assignNewUid(tsk);

    u = tsk->uid();

    if (internalAddRecord(tsk)) {
        needsSave = true;
        sort();

        updateJournal(*tsk, ACTION_ADD);

        {
            QtopiaIpcEnvelope e("QPE/PIM", "addedTask(int,QTask)");
            e << getpid();
            e << *tsk;
        }
    }
    return u;
}

bool QTaskXmlIO::removeTask(int row)
{
    return removeTask(task(row));
}

bool QTaskXmlIO::removeTask(const QUniqueId & id)
{
    return removeTask(task(id));
}

bool QTaskXmlIO::removeTasks(const QList<int> &rows)
{
    QList<QUniqueId> ids;
    foreach(int pos, rows) {
        ids.append(taskId(pos));
    }
    return removeTasks(ids);
}

bool QTaskXmlIO::removeTasks(const QList<QUniqueId> &ids)
{
    bool res = true;
    foreach(QUniqueId id, ids) {
        // can't really bail, so continue and get the ones that work.
        if (!removeTask(id))
            res = false;
    }
    return res;
}

bool QTaskXmlIO::removeTask(const QTask &task)
{
    if (accessMode() == ReadOnly)
        return false;

    QTask *tsk = new QTask((const QTask &)task);
    if (internalRemoveRecord(tsk)) {
        needsSave = true;
        sort();

        updateJournal(task, ACTION_REMOVE);

        {
            QtopiaIpcEnvelope e("QPE/PIM", "removedTask(int,QTask)");
            e << getpid();
            e << task;
        }
        return true;
    }
    return false;
}

bool QTaskXmlIO::updateTask(const QTask &task)
{
    if (accessMode() == ReadOnly)
        return false;

    QTask *tsk = new QTask((const QTask &)task);
    if (internalUpdateRecord(tsk)) {
        needsSave = true;
        sort();

        updateJournal(task, ACTION_REPLACE);

        {
            QtopiaIpcEnvelope e("QPE/PIM", "updatedTask(int,QTask)");
            e << getpid();
            e << task;
        }
        return true;
    }
    return false;
}

void QTaskXmlIO::ensureDataCurrent(bool forceReload)
{
    if (accessMode() == WriteOnly || ( isDataCurrent() && !forceReload) )
        return;

    m_Tasks.clear();
    m_Filtered.clear();
    loadData();
}

const char *QTaskXmlIO::recordStart() const
{
    static char *s = "<Task "; // No tr
    return s;
}

const char *QTaskXmlIO::listStart() const
{
    static char *s = "<!DOCTYPE Tasks>\n<Tasks>\n";
    return s;
}

const char *QTaskXmlIO::listEnd() const
{
    static char *s = "</Tasks>\n";
    return s;
}

QPimRecord *QTaskXmlIO::createRecord() const
{
    return new QTask();
}

bool QTaskXmlIO::select(const QTask &c) const
{
    if (cCompFilter && c.isCompleted())
        return false;
    return cFilter.accepted(c.categories());
}

QCategoryFilter QTaskXmlIO::categoryFilter() const
{
    return cFilter;
}

void QTaskXmlIO::setCategoryFilter(const QCategoryFilter & f)
{
    if (f != cFilter) {
        cFilter = f;
        m_Filtered.clear();
        foreach(QTask *tsk, m_Tasks) {
            if (select(*tsk))
                m_Filtered.append(tsk);
        }
        sort();
    }
}

bool QTaskXmlIO::completedFilter() const
{
    return cCompFilter;
}

void QTaskXmlIO::setCompletedFilter(bool f)
{
    if (f != cCompFilter) {
        cCompFilter = f;
        m_Filtered.clear();
        foreach(QTask *tsk, m_Tasks) {
            if (select(*tsk))
                m_Filtered.append(tsk);
        }
        sort();
    }
}

QTaskModel::Field QTaskXmlIO::field() const
{
    return cKey;
}

void QTaskXmlIO::setField(QTaskModel::Field key)
{
    if (key != cKey) {
        cKey = key;
        sort();
    }
}

QTask QTaskXmlIO::task( const QUniqueId & id ) const
{
    foreach(QTask *t, m_Tasks) {
        if (t->uid() == id)
            return QTask(*t);
    }
    return QTask();
}


QTask QTaskXmlIO::task(int pos) const
{
    return QTask(*(m_Filtered.at(pos)));
}

QUniqueId QTaskXmlIO::taskId(int pos) const
{
    return task(pos).uid();
}

int QTaskXmlIO::row(const QUniqueId & id) const
{
    for (int index = 0 ; index < m_Filtered.count(); ++index) {
        if (m_Filtered.at(index)->uid() == id) {
            return index;
        }
    }
    return -1;
}

int QTaskXmlIO::count() const
{
    return m_Filtered.count();
}

int QTaskXmlIO::startSearch(const QString &)
{
    // use filteredSearch??
    return -1;
}

int QTaskXmlIO::nextSearchItem()
{
    return -1;
}

void QTaskXmlIO::clearSearch()
{
}

QList<QUniqueId> QTaskXmlIO::filteredSearch(const QString &findString) const
{
    // its a sub string search.   static bool wrapAround = false;
    QRegExp r( findString );
    r.setCaseSensitivity( Qt::CaseInsensitive );

    QList<QUniqueId> results;
    for (int index = 0; index < m_Filtered.count(); ++index) {
        const QTask *t = m_Filtered.at(index);
        if (t->match(r)) {
            results.append(t->uid());
        }
    }
    return results;
}


