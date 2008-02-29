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
** Contact info\@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qfile.h>
#include <qasciidict.h>
#include "task.h"
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/stringutil.h>
#include <qfileinfo.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qapplication.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include "todoxmlio_p.h"
#include "task.h"
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

SortedTasks::SortedTasks()
    : SortedRecords<PimTask>()
{}


SortedTasks::SortedTasks(uint s)
    : SortedRecords<PimTask>(s)
{}

SortedTasks::~SortedTasks() {}

int SortedTasks::compareItems(QCollection::Item d1, QCollection::Item d2)
{
    PrTask *pt1 = (PrTask *)d1;
    PrTask *pt2 = (PrTask *)d2;

    int result = 0;
    if ( mKey != -1 )
	result = compareTaskField( (PimTask::TaskFields) mKey, pt1, pt2);
    if ( !result )
	result = compareTaskField( PimTask::Completed, pt1, pt2);
    if ( !result )
	result = compareTaskField( PimTask::Priority, pt1, pt2);
    if ( !result )
	result = compareTaskField( PimTask::DueDateYear, pt1, pt2);
    if ( !result )
	result = compareTaskField( PimTask::Description, pt1, pt2);

    if ( mAscending )
	result = -result;

    return result;
}

int SortedTasks::compareTaskField(int key, PrTask *pt1, PrTask *pt2)
{
    switch (key) {
	case PimTask::Completed:
	    {
		int c = 0;
		if (pt1->isCompleted())
		    c++;
		if (pt2->isCompleted())
		    c--;

		return c;
	    }
	case PimTask::Priority: return ( pt1->priority() - pt2->priority() );
	case PimTask::DueDateYear:
	    if (pt1->hasDueDate() && pt2->hasDueDate())
		return ( pt2->dueDate().daysTo(pt1->dueDate()) );
	    else if (pt1->hasDueDate())
		return -1;
	    else if (pt2->hasDueDate())
		return 1;

	    return 0;
	case PimTask::Description: return QString::compare(pt1->description().lower(), pt2->description().lower());
	case -1:
	default: return 0;
    }
}

TodoXmlIO::TodoXmlIO(AccessMode m,
		     const QString &file,
		     const QString &journal ) :
    TaskIO(m),
    PimXmlIO(PimTask::keyToIdentifierMap(), PimTask::identifierToKeyMap() ),
    cFilter(-2), cCompFilter(FALSE), needsSave(FALSE)
{
    if ( file != QString::null )
	setDataFilename( file );
    else setDataFilename( Global::applicationFileName( "todolist", "todolist.xml" ) );
    if ( journal != QString::null )
	setJournalFilename( journal );
    else setJournalFilename( Global::journalFileName( ".todojournal" ) );

    m_Tasks.setAutoDelete(TRUE);

    loadData();

    if (m == ReadOnly) {
#ifndef QT_NO_COP
	QCopChannel *channel = new QCopChannel( "QPE/PIM",  this );

	connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
		this, SLOT(pimMessage(const QCString&, const QByteArray&)) );

#endif
    }
}

void TodoXmlIO::pimMessage(const QCString &message, const QByteArray &data)
{
    QDataStream ds(data, IO_ReadOnly);
    if (message == "addedTask(int,PimTask)") {
	int pid;
	PimTask task;
	ds >> pid;
	ds >> task;
	if (pid != getpid()) {
	    internalAddRecord(new PimTask(task));
	    emit tasksUpdated();
	}
    } else if (message == "removedTask(int,PimTask)") {
	int pid;
	PimTask task;
	ds >> pid;
	ds >> task;
	if (pid != getpid()) {
	    internalRemoveRecord(new PimTask(task));
	    emit tasksUpdated();
	}
    } else if (message == "updatedTask(int,PimTask)") {
	int pid;
	PimTask task;
	ds >> pid;
	ds >> task;
	if (pid != getpid()) {
	    internalUpdateRecord(new PimTask(task));
	    emit tasksUpdated();
	}
    } else if (message == "reloadTasks()") {
        ensureDataCurrent();
    } else if ( message == "reload(int)" ) {
	int force;
	ds >> force;
        ensureDataCurrent(force);
    }
}

TodoXmlIO::~TodoXmlIO()
{
    if (accessMode() != ReadOnly )
	saveData();
}

bool TodoXmlIO::internalAddRecord(PimRecord *r)
{
    PrTask *todo = (PrTask *)r;
    m_Tasks.append( todo );
    if (select(*todo))
	m_Filtered.append(todo);

    return TRUE;
}

bool TodoXmlIO::internalRemoveRecord(PimRecord *r)
{
    PrTask *todo = (PrTask *)r;
    for (m_Tasks.first(); m_Tasks.current(); m_Tasks.next()) {
	if (m_Tasks.current()->uid() == todo->uid()) {
	    if ( select( *(m_Tasks.current()) ) ) {
		m_Filtered.remove(m_Tasks.current());
	    }
	    m_Tasks.remove();
	    delete todo;
	    return TRUE;
	}
    }
    delete todo;
    return FALSE;
}

bool TodoXmlIO::internalUpdateRecord(PimRecord *r)
{
    PrTask *todo = (PrTask *)r;
    for (m_Tasks.first(); m_Tasks.current(); m_Tasks.next()) {
	PrTask *current = m_Tasks.current();
	if (current->uid() == todo->uid()) {
	    if ( select(*current) ) {
		m_Filtered.remove(current);
	    }

	    if ( current != todo ) {
		*current = *todo;
		delete todo;
	    }

	    if (select(*current)) {
		m_Filtered.append(current);
	    }
	    return TRUE;
	}
    }
    delete todo;
    return FALSE;
}

TaskIteratorMachine *TodoXmlIO::begin() const
{
    return new TodoXmlIterator(m_Tasks);
}

/**
 * Returns the full task list.  This is guaranteed
 * to be current against what is stored by other apps.
 */
const QList<PrTask>& TodoXmlIO::tasks() {
  ensureDataCurrent();
  return m_Tasks;
}

const SortedTasks& TodoXmlIO::sortedTasks() {
    ensureDataCurrent();
    return m_Filtered;
}


PrTask TodoXmlIO::taskForId( const QUuid &u, bool *ok ) const
{
    QListIterator<PrTask> it(m_Tasks);

    PrTask *p;
    for (; it.current(); ++it ) {
	p = *it;
	if (u == p->uid()) {
	    if (ok)
		*ok = TRUE;
	    return *p;
	}
    }

    if (ok)
	*ok = FALSE;
    return PrTask();
}

void TodoXmlIO::clear()
{
    cFilter = -2;
    m_Filtered.clear();
    m_Tasks.clear();
    needsSave = TRUE;
}

/**
 * Saves the current task data.  Returns true if
 * successful.
 */
bool TodoXmlIO::saveData()
{
    if ( !QFile::exists( dataFilename() ) )
	needsSave = TRUE;

    if (!needsSave)
	return TRUE;
    if (accessMode() != ReadOnly) {
	if (PimXmlIO::saveData((QList<PimRecord> &)m_Tasks)) {
	    needsSave = FALSE;
	    return TRUE;
	}
    }
    return FALSE;
}

bool TodoXmlIO::loadData()
{
    if (PimXmlIO::loadData()) {
	m_Filtered.sort();
	emit tasksUpdated();
	return TRUE;
    }
    return FALSE;
}


QString TodoXmlIO::recordToXml(const PimRecord *p)
{
    const PrTask *e = (const PrTask *)p;

    QMap<int,QString> data = p->fields();

    bool hasDueDate = e->hasDueDate();

    const QMap<int,QCString> keyToIdentifier = PimTask::keyToIdentifierMap();
    QString out;
    for ( QMap<int, QString>::ConstIterator fit = data.begin();
	    fit != data.end(); ++fit ) {

	int key = fit.key();
	if ( !hasDueDate ) {
	    if ( key == PimTask::DueDateYear || key == PimTask::DueDateMonth || key == PimTask::DueDateDay )
		continue;
	}

	const QString &value = fit.data();
	if ( !value.isEmpty() ) {
	    out += keyToIdentifier[key];
	    out += "=\"" + Qtopia::escapeString(value) + "\" ";
	}
    }

    out += customToXml( p );

    return out;
}

QUuid TodoXmlIO::addTask(const PimTask &task, bool assignUid )
{
    QUuid u;
    if (accessMode() == ReadOnly)
	return u;

    PrTask *tsk = new PrTask((const PrTask &)task);

    if ( assignUid || tsk->uid().isNull() )
	assignNewUid(tsk);

    u = tsk->uid();

    if (internalAddRecord(tsk)) {
	needsSave = TRUE;
	m_Filtered.sort();

	updateJournal(*tsk, ACTION_ADD);

	{
#ifndef QT_NO_COP
	    QCopEnvelope e("QPE/PIM", "addedTask(int,PimTask)");
	    e << getpid();
	    e << *tsk;
#endif
	}
    }
    return u;
}

void TodoXmlIO::removeTask(const PimTask &task)
{
    if (accessMode() == ReadOnly)
	return;

    PrTask *tsk = new PrTask((const PrTask &)task);
    if (internalRemoveRecord(tsk)) {
	needsSave = TRUE;
	m_Filtered.sort();

	updateJournal(task, ACTION_REMOVE);

	{
#ifndef QT_NO_COP
	    QCopEnvelope e("QPE/PIM", "removedTask(int,PimTask)");
	    e << getpid();
	    e << task;
#endif
	}
    }
}

void TodoXmlIO::updateTask(const PimTask &task)
{
    if (accessMode() == ReadOnly)
	return;

    PrTask *tsk = new PrTask((const PrTask &)task);
    if (internalUpdateRecord(tsk)) {
	needsSave = TRUE;
	m_Filtered.sort();

	updateJournal(task, ACTION_REPLACE);

	{
#ifndef QT_NO_COP
	    QCopEnvelope e("QPE/PIM", "updatedTask(int,PimTask)");
	    e << getpid();
	    e << task;
#endif
	}
    }
}

void TodoXmlIO::ensureDataCurrent(bool forceReload)
{
    if (accessMode() == WriteOnly || ( isDataCurrent() && !forceReload) )
	return;

    m_Tasks.clear();
    m_Filtered.clear();
    loadData();
}

const char *TodoXmlIO::recordStart() const
{
    static char *s = "<Task "; // No tr
    return s;
}

const char *TodoXmlIO::listStart() const
{
    static char *s = "<!DOCTYPE Tasks>\n<Tasks>\n";
    return s;
}

const char *TodoXmlIO::listEnd() const
{
    static char *s = "</Tasks>\n";
    return s;
}

PimRecord *TodoXmlIO::createRecord() const
{
    return new PrTask();
}

bool TodoXmlIO::select(const PrTask &c) const
{
    if (cFilter == -3 || cCompFilter && c.isCompleted())
	return FALSE;

    if (cFilter == -2)
	return TRUE;

    QArray<int> cats = c.categories();
    if ( cFilter == -1 ) {
	if ( cats.count() > 0 )
	    return FALSE;
    } else {
	if (cats.find(cFilter) == -1)
	    return FALSE;
    }
    return TRUE;
}

int TodoXmlIO::filter() const
{
    return cFilter;
}

void TodoXmlIO::setFilter(int f)
{
    if (f != cFilter) {
	cFilter = f;
	m_Filtered.clear();
	for (m_Tasks.first(); m_Tasks.current(); m_Tasks.next()) {
	    PrTask *cnt = m_Tasks.current();
	    if (select(*cnt)) {
		m_Filtered.append(cnt);
	    }
	}
	m_Filtered.sort();
    }
}

bool TodoXmlIO::completedFilter() const
{
    return cCompFilter;
}

void TodoXmlIO::setCompletedFilter(bool f)
{
    if (f != cCompFilter) {
	cCompFilter = f;
	m_Filtered.clear();
	for (m_Tasks.first(); m_Tasks.current(); m_Tasks.next()) {
	    PrTask *cnt = m_Tasks.current();
	    if (select(*cnt)) {
		m_Filtered.append(cnt);
	    }
	}
	m_Filtered.sort();
    }
}

int TodoXmlIO::sortKey() const
{
    return m_Filtered.sortKey();
}

bool TodoXmlIO::sortAcending() const
{
    return m_Filtered.ascending();
}

void TodoXmlIO::setSorting(int key, bool ascending)
{
    if (key != m_Filtered.sortKey() || ascending != m_Filtered.ascending() ) {
	m_Filtered.setSorting( key, ascending );
	m_Filtered.sort();
    }
}

