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
#include <qpe/config.h>
#include <qpe/global.h>
#include <qpe/stringutil.h>
#include <qfileinfo.h>
#include <qpe/qcopenvelope_qws.h>
#include <qapplication.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include "todoxmlio_p.h"

SortedTasks::SortedTasks()
    : SortedRecords<PimTask>(), so(Completed)
{}


SortedTasks::SortedTasks(uint s)
    : SortedRecords<PimTask>(s), so(Completed)
{}

SortedTasks::~SortedTasks() {}

int SortedTasks::compareItems(QCollection::Item d1, QCollection::Item d2) 
{
    PrTask *pt1 = (PrTask *)d1;
    PrTask *pt2 = (PrTask *)d2;

    int p, d, t, c;

    p =  pt1->priority() - pt2->priority();

    if (pt1->hasDueDate() && pt2->hasDueDate()) 
	d =  pt2->dueDate().daysTo(pt1->dueDate());
    else if (pt1->hasDueDate())
	d = -1;
    else if (pt2->hasDueDate())
	d = 1;
    else
	d = 0;
  
    t = QString::compare(pt1->description(), pt2->description());

    c = 0;
    if (pt1->isCompleted())
	c++;
    if (pt2->isCompleted())
	c--;

    switch (so) {
	case Completed:
	    if (c)
		return c;
	    break;
	case Priority:
	    if (p)
		return p;
	    break;
	case DueDate:
	    if (d)
		return d;
	    break;
	case Description:
	    if (t)
		return t;
	    break;
    }

    // default order;
    if (c)
	return c;
    if (p)
	return p;
    if (d)
	return d;
    return t;
}


void SortedTasks::setSortOrder(SortOrder s)
{
    so = s;
    setDirty();
}

SortedTasks::SortOrder SortedTasks::sortOrder() const
{
    return so;
}

TodoXmlIO::TodoXmlIO(AccessMode m) : TaskIO(m), cFilter(-2), cCompFilter(FALSE), dict(10), needsSave(FALSE)
{
    m_Tasks.setAutoDelete(TRUE);

    dict.setAutoDelete( TRUE );
    dict.insert( "Completed", new int(FCompleted) );
    dict.insert( "HasDate", new int(FHasDate) );
    dict.insert( "Priority", new int(FPriority) );
    dict.insert( "Categories", new int(FCategories) );
    dict.insert( "Description", new int(FDescription) );
    dict.insert( "DateYear", new int(FDateYear) );
    dict.insert( "DateMonth", new int(FDateMonth) );
    dict.insert( "DateDay", new int(FDateDay) );
    dict.insert( "Uid", new int(FUid) );

    ensureDataCurrent();

    if (m == ReadOnly) {
	QCopChannel *channel = new QCopChannel( "QPE/PIM",  this );

	connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
		this, SLOT(pimMessage(const QCString&, const QByteArray&)) );

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
    }
}

void TodoXmlIO::assignField(PimRecord *rec, const QCString &attr, const QString &value)
{
    static int dtY = 0;
    static int dtM = 0;
    static int dtD = 0;
    static PimRecord *prevrec = 0;

    if (rec != prevrec) {
	dtY = 0;
	dtM = 0;
	dtD = 0;
	prevrec = rec;
    }

    PrTask *todo = (PrTask *)rec;

    int *lookup = dict[ attr.data() ];
    if ( !lookup ) {
	todo->setCustomField(attr, value);
	return;
    }

    switch( *lookup ) {
	case FCompleted:
	    todo->setCompleted( value.toInt() );
	    break;
	case FHasDate:
	    // leave...
	    // currently default is no due date, setting true is
	    // done by actually getting a due date.
	    //hasDueDate = value.toInt();
	    break;
	case FPriority:
	    todo->setPriority( value.toInt() );
	    break;
	case FCategories:
	    todo->setCategories( idsFromString( value ) );
	    break;
	case FDescription:
	    todo->setDescription( value );
	    break;
	case FDateYear:
	    dtY = value.toInt();
	    break;
	case FDateMonth:
	    dtM = value.toInt();
	    break;
	case FDateDay:
	    dtD = value.toInt();
	    break;
	case FUid:
	    setUid(*todo, uuidFromInt(value.toInt()));
	    break;
	default:
	    qDebug( "TodoXmlIO::assignField(): Missing attribute: %s", attr.data() );
	    break;
    }

    if ( dtY != 0 && dtM != 0 && dtD != 0 ) {
	todo->setDueDate( QDate( dtY, dtM, dtD) );
    }
}


TodoXmlIO::~TodoXmlIO() 
{
    if (accessMode() == ReadWrite)
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

	    m_Tasks.remove();
	    m_Tasks.append(todo);

	    if (select(*todo)) {
		m_Filtered.append(todo);
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
QList<PrTask>& TodoXmlIO::tasks() {
  ensureDataCurrent();
  return m_Tasks;    
}

/**
 * Returns the filtered task list.  This is guaranteed
 * to be current against what is stored by other apps.
 */
const SortedTasks& TodoXmlIO::sortedTasks() {
  ensureDataCurrent();
  return m_Filtered;    
}

/**
 * Saves the current task data.  Returns true if
 * successful.
 */
bool TodoXmlIO::saveData() 
{
    if (!needsSave)
	return TRUE;
    if (accessMode() == ReadWrite) {
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
	return TRUE;
    }
    return FALSE;
}

QString TodoXmlIO::recordToXml(const PimRecord *rec) 
{
    const PrTask *task = (const PrTask *)rec;

    QString buf;

    buf += "Completed=\"";
    buf += QString::number( (int)task->isCompleted() );
    buf += "\"";
    buf += " HasDate=\"";
    buf += QString::number( (int)task->hasDueDate() );
    buf += "\"";
    buf += " Priority=\"";
    buf += QString::number( (int)task->priority() );
    buf += "\"";
    buf += " Categories=\"";
    buf += idsToString( task->categories() );
    buf += "\"";
    buf += " Description=\"";
    buf += Qtopia::escapeString( task->description() );
    buf += "\"";
    if ( task->hasDueDate() ) {
	QDate mDDate = task->dueDate();
	buf += " DateYear=\"";
	buf += QString::number( mDDate.year() );
	buf += "\"";
	buf += " DateMonth=\"";
	buf += QString::number( mDDate.month() );
	buf += "\"";
	buf += " DateDay=\"";
	buf += QString::number( mDDate.day() );
	buf += "\"";
    }
    buf += " Uid=\"";
    buf += QString::number( uuidToInt(task->uid()) );
    // terminate it in the application...
    buf += "\"";

    buf += customToXml(task);
    return buf;
}
  
void TodoXmlIO::addTask(const PimTask &task)
{
    if (accessMode() != ReadWrite)
	return;

    PrTask *tsk = new PrTask((const PrTask &)task);
    assignNewUid(tsk);
    if (internalAddRecord(tsk)) {
	needsSave = TRUE;
	m_Filtered.sort();

	updateJournal(*tsk, ACTION_ADD);

	{
	    QCopEnvelope e("QPE/PIM", "addedTask(int,PimTask)"); 
	    e << getpid();
	    e << *tsk;
	}
    }
}

void TodoXmlIO::removeTask(const PimTask &task)
{
    if (accessMode() != ReadWrite)
	return;

    PrTask *tsk = new PrTask((const PrTask &)task);
    if (internalRemoveRecord(tsk)) {
	needsSave = TRUE;
	m_Filtered.sort();

	updateJournal(task, ACTION_REMOVE);

	{
	    QCopEnvelope e("QPE/PIM", "removedTask(int,PimTask)"); 
	    e << getpid();
	    e << task;
	}
    }
}

void TodoXmlIO::updateTask(const PimTask &task)
{
    if (accessMode() != ReadWrite)
	return;

    PrTask *tsk = new PrTask((const PrTask &)task);
    if (internalUpdateRecord(tsk)) {
	needsSave = TRUE;
	m_Filtered.sort();

	updateJournal(task, ACTION_REPLACE);

	{
	    QCopEnvelope e("QPE/PIM", "updatedTask(int,PimTask)"); 
	    e << getpid();
	    e << task;
	}
    }
}

void TodoXmlIO::ensureDataCurrent(bool forceReload) {
  if (isDataCurrent() && !forceReload)
    return;
  
  m_Tasks.clear();
  m_Filtered.clear();
  loadData();
}

const QString TodoXmlIO::dataFilename() const {
  QString filename = Global::applicationFileName("todolist",
						 "todolist.xml");
  return filename;
}

const QString TodoXmlIO::journalFilename() const {
  QString str = getenv("HOME");
  str +="/.todojournal";
  return str;
}

const char *TodoXmlIO::recordStart() const
{
    static char *s = "<Task ";
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
    if (cCompFilter && c.isCompleted())
	return FALSE;

    QArray<int> cats = c.categories();
    if ( cFilter == -1 ) {
	if ( cats.count() > 0 )
	    return FALSE;
    }  else if ( cFilter != -2 ) {
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

SortedTasks::SortOrder TodoXmlIO::sortOrder() const
{
    return m_Filtered.sortOrder();
}

void TodoXmlIO::setSortOrder( SortedTasks::SortOrder so )
{
    if (so != m_Filtered.sortOrder() ) {
	m_Filtered.setSortOrder(so);
	m_Filtered.sort();
    }
}

