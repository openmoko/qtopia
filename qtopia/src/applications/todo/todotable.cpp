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

#include "todotable.h"

#include <qtopia/categoryselect.h>
#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/timestring.h>
#ifdef QTOPIA_PHONE
# include <qtopia/contextbar.h>
#endif
#include <qtopia/global.h>
#include <qtopia/resource.h>

#include <qasciidict.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <qfile.h>
#include <qpainter.h>
#include <qtextcodec.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qstyle.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qwidget.h>

#include <qcursor.h>
#include <qregexp.h>

#ifdef QTOPIA_DESKTOP
#include <qdconfig.h>
#include <qsettings.h>
#endif

#include <errno.h>
#include <stdlib.h>

static const int cacheshift = 5; // cachsize = 1 << cacheshift
//static const int cacheshift = 13;

static int BoxSize = 14;
static int RowHeight = 20;

static Qt::ButtonState bState = Qt::NoButton;
static int selectionBeginRow = -1;
static int selectionEndRow = -1;

static QString applicationPath;
static bool constructorDone = FALSE;


class TablePrivate
{
public:
    TablePrivate()
    {
	mEditedField = -1;
	mModified = FALSE;
	scrollButton = 0;
	wEditor = 0;
    }

    QUuid modifiedTask() { return mUid; }
    void clearModifiedTask()
    {
	mModified = FALSE;
	if ( scrollButton )
	    scrollButton->hide();
    }

    QPushButton *cornerButton(QWidget *parent)
    {
	if ( !scrollButton ) {
	    scrollButton = new QPushButton(parent);
	    scrollButton->setPixmap( Resource::loadPixmap("todolist/menu") );
	    scrollButton->hide();
	}

	return scrollButton;
    }

    void setModifiedTask(const QUuid &id, int field)
    {
	mUid = id;
	mEditedField = field;
	mModified = TRUE;
	if ( scrollButton )
	    scrollButton->show();
    }

    int editedTaskField() { return mEditedField; }
    bool hasModifiedTask() { return mModified; }

    /*	EditorWidget */
    void setEditorWidget(QWidget *w, int row, int col)
    {
	wEditor = w;
	editRow = row;
	editCol = col;
    }

    void clearEditorWidget()
    {
	wEditor = 0;
	editRow = -1;
	editCol = -1;
    }

    QWidget *editorWidget() { return wEditor; }
    int editorColumn() { return editCol; }
    int editorRow() { return editRow; }

private:
    int mEditedField;
    QUuid mUid;
    bool mModified;
    QPushButton *scrollButton;

    QWidget *wEditor;
    int editCol, editRow;
};

TodoTable::TodoTable(QWidget *parent, const char *name, const char *appPath , bool readonly )
    : QTable( 0, 0, parent, name ),
      mCat( 0 ),
      mLastRowShown( -1 ),
      le(0), ro(readonly)
{
    // value list iterators are uninialized until set to something.
    // need to set in case next is called before find.
    currFind = searchResults.end();
#ifdef QTOPIA_PHONE
    setShowGrid(FALSE);
#endif
    applicationPath = appPath;

    paletteChange(palette());

    QFont f = font();
    QFontMetrics fm(f);
    RowHeight = QMAX(20, fm.height() + 2);

    // keep it an even number, a bit over 3 / 4 of rowHeight.
    BoxSize =  ( RowHeight * 3 ) / 4;
    BoxSize += BoxSize % 2;
    
    // do not!! put the below assignment in the constructor init, as the todoplugin for
    // qtopiadesktop fails to resolve the symbol
    mTasks = new TodoXmlIO(ro ? TaskIO::ReadOnly : TaskIO::ReadWrite);
    mSel = NoSelection;

    d = new TablePrivate();

#ifndef QTOPIA_PHONE
    setCornerWidget( d->cornerButton(this) );
    cornerWidget()->hide();
    connect( d->cornerButton(this), SIGNAL( clicked() ),
	    this, SLOT( cornerButtonClicked() ) );
#endif

    mCat.load( categoryFileName() );
    setSorting( TRUE );
    QTable::setSelectionMode( QTable::NoSelection );

    setLeftMargin( 0 );
#ifndef QTOPIA_DESKTOP
    setFrameStyle( NoFrame );
#endif
    verticalHeader()->hide();
    horizontalHeader()->hide();
#ifdef QTOPIA_PHONE
    horizontalHeader()->setResizeEnabled(FALSE);
    horizontalHeader()->setClickEnabled(FALSE);
#endif

    connect(horizontalHeader(), SIGNAL(clicked(int)), this, SLOT(headerClicked(int)) );
    mSortColumn = 0;
    ascSort = FALSE;

    connect( this, SIGNAL( clicked(int,int,int,const QPoint&) ),
	     this, SLOT( activateCell(int,int,int,const QPoint&) ) );
    connect( this, SIGNAL( pressed(int,int,int,const QPoint&) ),
	     this, SLOT( startMenuTimer(int,int,int,const QPoint&) ) );
    connect( this, SIGNAL( doubleClicked(int,int,int,const QPoint&) ),
	     this, SIGNAL( doubleClicked() ) );
    connect( this, SIGNAL( currentChanged(int,int) ),
             this, SLOT( refreshCell(int,int) ) );

    //connect( &ta, SIGNAL( todolistUpdated() ), this, SLOT(refresh()));

    menuTimer = new QTimer( this );
    connect( menuTimer, SIGNAL(timeout()), this, SIGNAL(pressed()) );

#ifdef QTOPIA_DESKTOP
    readSettings();
#else
    QTimer::singleShot(0, this, SLOT(readSettings()) );
#endif
}

TodoTable::~TodoTable()
{
    saveSettings();
    delete d;
}

void TodoTable::setSelectionMode(SelectionMode m)
{
    if ( m == NoSelection && mSel != m ) {
	selectionBeginRow = -1;
	selectionEndRow = -1;
	refresh();
    }

    mSel = m;
}

QValueList<int> TodoTable::selectedTasks()
{
    QValueList<int> list;
    if ( mSel == Single ) {
    	if ( hasCurrentEntry() )
	    list.append( TodoXmlIO::uuidToInt(currentEntry().uid()) );
    } else if ( mSel == Extended ) {
	int fromRow = QMIN(selectionBeginRow, selectionEndRow);
	int toRow = QMAX(selectionBeginRow, selectionEndRow);
	if (toRow != -1 && fromRow != -1) {
	    int row = fromRow;
	    while(row <= toRow) {
		list.append(TodoXmlIO::uuidToInt(mTasks->filteredItem(row).uid()));
		++row;
	    }
	}

	// set current entry as selected when none is selected
	if ( !list.count() && hasCurrentEntry() )
	    list.append( TodoXmlIO::uuidToInt(currentEntry().uid()) );
    }

    return list;
}

/* not as bad as it looks
   value lists are implicitly shared.
*/
QValueList<PimTask> TodoTable::selected()
{
    QValueList<PimTask> list;
    if ( mSel == Single ) {
    	if ( hasCurrentEntry() )
	    list.append( currentEntry() );
    } else if ( mSel == Extended ) {
	int fromRow = QMIN(selectionBeginRow, selectionEndRow);
	int toRow = QMAX(selectionBeginRow, selectionEndRow);
	if (toRow != -1 && fromRow != -1)  {
	    int row = fromRow;
	    while(row <= toRow) {
		PimTask t = mTasks->filteredItem(row);
		if (!t.uid().isNull()) 
		    list.append( t );
		++row;
	    }
	}
	
	// set current entry as selected when none is selected
	if ( !list.count() && hasCurrentEntry() )
	    list.append( currentEntry() );
    }

    return list;

}

void TodoTable::selectAll()
{
    setSelection(0, mFiltered.size() - 1);
    refresh();
}

// We clear the selection and loop through all the rows since currentChanged
// will skip some rows if you move the pointing device fast enough
void TodoTable::setSelection(int fromRow, int toRow)
{
    if ( mSel == NoSelection || mSel == Single )
	return;


    selectionBeginRow = fromRow;
    selectionEndRow = toRow;
}

#ifdef Q_OS_UNIX
#include <sys/time.h>
#endif

void TodoTable::saveData(bool force)
{
    if (!ro) {
	if (!mTasks->saveData(force))
	    QMessageBox::information( this, tr( "Tasks" ),
		    tr("<qt>Device full.  Some changes may not be saved.</qt>"));
    }
}

void TodoTable::loadData()
{
    mTasks->ensureDataCurrent(TRUE);
}

bool TodoTable::categoriesChanged(const QArray<int> &cats)
{
    bool changed = FALSE;
    QListIterator<PrTask> it(mTasks->tasks() );
    for(; it.current(); ++it) {
	PimTask t( *(*it) );
	if ( t.pruneDeadCategories( cats ) ) {
    	    mTasks->updateTask( t );
	    changed = TRUE;
	}
    }
    return changed;
}

void TodoTable::reload()
{
    // If the user applies outside filters we have to clear our internal state
    if ( d->editorWidget() ) {
	endEdit(d->editorRow(), d->editorColumn(), FALSE, TRUE );
	d->clearEditorWidget();
    }

    selectionBeginRow = -1;
    selectionEndRow = -1;

    /*
       Shouldnt' need to do this, only should need to do if sorting changes.
    */
    if ( mSortColumn > -1 ) {
	mTasks->setSorting( headerKeyFields[ mSortColumn ] , ascSort );
    }
#ifdef Q_OS_UNIX
    struct timeval t1, t2, t3;

    gettimeofday(&t1, 0);
#endif

    mFiltered.resize(mTasks->filteredCount());

#ifdef Q_OS_UNIX
    gettimeofday(&t2, 0);
#endif

    mFilteredLoaded.resize(1 + (mFiltered.size() >> cacheshift));
    for (int i = 0; i < (int)mFilteredLoaded.size(); i++)
	mFilteredLoaded[i] = FALSE;

#ifdef Q_OS_UNIX
    gettimeofday(&t3, 0);

    //qDebug("TIMING %ld:%ld, %ld:%ld, %ld:%ld", t1.tv_sec, t1.tv_usec,
	    //t2.tv_sec, t2.tv_usec,
	    //t3.tv_sec, t3.tv_usec);
#endif

    refresh();
    setFocus();	// in case of inline widgets grabbing focus
    emit currentChanged();
}

int TodoTable::pos(const int id)
{
    int res = mTasks->filteredPos(id);
    return res;
}

class PriorityEdit : public QListBox 
{

    Q_OBJECT

public:
    PriorityEdit(QWidget *parent)
	: QListBox(parent, "priorityedit", WType_Popup)
	{
	    setAutoScrollBar( FALSE );
	    setBottomScrollBar( FALSE );
	    setAutoBottomScrollBar( FALSE );
	    // populate
	    insertItem( "1" );
	    insertItem( "2" );
	    insertItem( "3" );
	    insertItem( "4" );
	    insertItem( "5" );
	    connect(this, SIGNAL(clicked(QListBoxItem*)), this, SLOT(internalSelectPriority()));
	}

    int currentPriority() const { return currentItem() + 1; }

signals:
    void cancelEdit();

private slots:
    void internalSelectPriority() {
	emit selected(currentItem());
    }

protected:
    void keyPressEvent(QKeyEvent *e) {
	switch (e->key()) {
#ifdef QTOPIA_PHONE
	    case Key_No:
	    case Key_Back:
#endif
	    case Key_Escape:
		emit cancelEdit();
		break;
	    default:
		QListBox::keyPressEvent(e);
	}
    }
};

QWidget *TodoTable::priorityEdit(int row, int col)
{
    if (!le) {
	le = new PriorityEdit(this);//, "priority_edit", WType_Popup);

	connect(le, SIGNAL(selected(int)), this, SLOT(delaySetCellContentFromEditor()));
	connect(le, SIGNAL(cancelEdit()), this, SLOT(delayCancelEdit()));
    }

    le->setCurrentItem( mTasks->filteredItem(row).priority() - 1 );
    //size

    QWidget *desktop = QApplication::desktop();
    int sh = desktop->height();			// screen height

    int x = 0;
    int y = 0;
    QPoint pos(columnPos(col), rowPos(row+1));//, x, y);
    // contentsToViewport seems buggy, always added 2000 to 
    // each number for some strange reason.
    //pos = contentsToViewport(pos);
    pos = QPoint( pos.x() - contentsX(),
		   pos.y() - contentsY() );
    pos += QPoint(0, horizontalHeader()->height());
    pos = mapToGlobal( pos );

    // ### Similar code is in QPopupMenu
    le->resize(columnWidth(col) + le->frameWidth() * 2, 
	    5 * le->item( 0 )->height(le)
	    + le->frameWidth() * 2);
    x = pos.x();
    y = pos.y();
    int h = le->height();

    if (y + h > sh ) {
	y -= h + rowHeight(row) - 2*le->frameWidth();
    }
    le->move(x,y);

    //show
    le->raise();
    le->show();

    //connect

    return le;
}

void TodoTable::activateCell( int row, int col, int , const QPoint &pos )
{
    if ( d->editorWidget() ) {
	endEdit(d->editorRow(), d->editorColumn(), FALSE, TRUE );
	d->clearEditorWidget();
    }

    if ( !cellGeometry( row, col ).contains(pos) )
	return;
    if (row < 0 || row >= (int)mFiltered.size())
	return;

    PimTask task = mTasks->filteredItem(row);

    if (ro) {
	menuTimer->stop();
	emit clicked();
    } else {
	int field = headerKeyFields[ currentColumn() ];
	switch ( field )
	{
	    case PimTask::CompletedField:
		{
		    int x = pos.x() - columnPos( col );
		    int y = pos.y() - rowPos( row );
		    int w = columnWidth( col );
		    int h = rowHeight( row );
		    if ( x >= ( w - BoxSize ) / 2 && x <= ( w - BoxSize ) / 2 + BoxSize &&
			    y >= ( h - BoxSize ) / 2 && y <= ( h - BoxSize ) / 2 + BoxSize ) {
			toggleTaskCompletion(row);
			return;
		    }
		}
		break;
	    case PimTask::Priority:
		{
		    QWidget *w = priorityEdit(row, col);
		    d->setEditorWidget(w, row, col );
		    return;
		}
		break;
	    default:
		menuTimer->stop();
		emit clicked();
		break;
	}
    }
}

void TodoTable::toggleTaskCompletion(int row)
{
    PimTask task= mTasks->filteredItem(row);
// Go to 'next' when marking done. A bit confusing.
#ifdef QTOPIA_TODOLIST_NEXT_UPON_DONE
    QUuid n = mTasks->filteredItem((row+1)%numRows()).uid();
#endif
    bool c = !task.isCompleted();
    task.setCompleted(c);
    if ( c )
	task.setCompletedDate(QDate::currentDate());
    d->setModifiedTask(task.uid(), PimTask::CompletedField);
    emit updateTask(task);
#ifdef QTOPIA_TODOLIST_NEXT_UPON_DONE
    setCurrentEntry(n);
#endif
}

void TodoTable::keyPressEvent( QKeyEvent *e )
{
    if ( d->editorWidget() ) {
	endEdit(d->editorRow(), d->editorColumn(), FALSE, TRUE );
	d->clearEditorWidget();
	setFocus();
    }

    int col  = currentColumn();
    int row = currentRow();

    if (row < 0 || row >= (int)mFiltered.size()) {
	QTable::keyPressEvent( e );
	return;
    }

#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() ) {
	switch( e->key() ) {
	    case Qt::Key_Select:
		if ( !isModalEditing() ) {
		    QTable::keyPressEvent( e );
		    break;
		}
	    case Qt::Key_Space:
	    case Qt::Key_Return:
	    case Qt::Key_Enter:
		if (ro) {
		    emit clicked();
		} else {
		    if (headerKeyFields[col] ==  PimTask::CompletedField) {
			toggleTaskCompletion(row);

		    } else if (headerKeyFields[col] ==  PimTask::Priority) {
			QWidget *w = priorityEdit(row, col);
			d->setEditorWidget(w, row, col );
			return;
		    } else {
			emit clicked();
		    }
		}
		break;
	    default:
		QTable::keyPressEvent( e );
	}
    } else
#endif
	// non phone or touchscreen phone
    if ( e->key() == Key_Space || e->key() == Key_Return
#ifdef QTOPIA_PHONE
	|| e->key() == Key_Select
#endif
	|| e->key() == Key_Enter ) {
	if (ro) {
	    emit clicked();
	} else {
	    int field = headerKeyFields[ col ];
	    switch ( field ) {
		case PimTask::CompletedField:
		    {
			toggleTaskCompletion(row);
			return;
		    }
		    break;
		case PimTask::Priority:
		    {
			QWidget *w = priorityEdit(row, col);
			d->setEditorWidget(w, row, col );
			return;
		    }
		    break;
		default:
		    emit clicked();
		    break;
	    }
	}
    } else {
	QTable::keyPressEvent( e );
    }
}

void TodoTable::startMenuTimer( int row, int col, int, const QPoint &pos )
{
    if ( col == 2 && cellGeometry( row, col ).contains(pos) )
	menuTimer->start( 750, TRUE );
}

void TodoTable::refreshCell( int row, int )
{
//    qDebug("slotCurrentChanged %d", row );
    bool needRefresh = d->hasModifiedTask();

    if ( mSel == Extended ) {
	if ( (bState & Qt::LeftButton) ) {
	    if ( selectionBeginRow == -1 ) {
		selectionBeginRow = row;
	    } else {
		selectionEndRow = row;
	    }
	    needRefresh = TRUE;
	} else if (selectionBeginRow != -1 || selectionEndRow != -1) {
	    selectionBeginRow = -1;
	    selectionEndRow = -1;
	    needRefresh = TRUE;
	}
    }
    
    if ( needRefresh ) {
	d->clearModifiedTask();
	refresh();
    }

    menuTimer->stop();
    emit currentChanged();
}

bool TodoTable::hasCurrentEntry()
{
    return mFiltered.size() != 0;
}

PimTask TodoTable::currentEntry()
{
    // if count is a long op, get it once damn it.
    int fcount = (int)mFiltered.size();
    if (fcount == 0)
	return PimTask();
    if (currentRow() >= fcount) {
	setCurrentCell(fcount - 1, currentColumn());
    } else if (currentRow() < 0) {
	setCurrentCell(0, currentColumn());
    }
    return mTasks->filteredItem(currentRow());
}

void TodoTable::setCurrentEntry(const QUuid &uid)
{
    int index = mTasks->filteredPos(TodoXmlIO::uuidToInt(uid));
    if (index >= 0)
	setCurrentCell(index, currentColumn());
}

#ifdef QTOPIA_PHONE
void TodoTable::setCurrentCell(int r, int c)
{
    if (ro) {
	int oldrow = currentRow();
	if (r == oldrow)
	    return;
	for (int i = 0; i < numCols(); ++i) {
	    updateCell(oldrow, i);
	    updateCell(r, i);
	}
	QTable::setCurrentCell(r, 0);
    } else {
	if (headerKeyFields[c] == PimTask::CompletedField || headerKeyFields[c] == PimTask::Priority)
	    ContextBar::setLabel(this, Qt::Key_Select, ContextBar::Select);
	else 
	    ContextBar::setLabel(this, Qt::Key_Select, ContextBar::View);
	QTable::setCurrentCell(r,c);
    }
}
#endif

void TodoTable::setCompletedFilter(bool b)
{
    mTasks->setCompletedFilter(b);
    // refresh.
}

bool TodoTable::completedFilter() const
{
    return mTasks->completedFilter();
    // refresh.
}

void TodoTable::setFilter(int b)
{
    mTasks->setFilter(b);
    // refresh.
}

int TodoTable::filter() const
{
    return mTasks->filter();
    // refresh.
}

void TodoTable::addEntry( const PimTask &todo, bool auid )
{
    if (ro)
	return;
    bool before = verticalScrollBar()->isVisible();
    QUuid uid = mTasks->addTask( todo, auid );
    reload();
    setCurrentEntry( uid );
    bool after =  verticalScrollBar()->isVisible();
    if (before != after) {
	fitHeadersToWidth();
    }
}

void TodoTable::removeEntry(const PimTask &todo )
{
    if (ro)
	return;
    bool before = verticalScrollBar()->isVisible();
    mTasks->removeTask( todo );
    reload();
    bool after = verticalScrollBar()->isVisible();
    if (before != after) {
	fitHeadersToWidth();
    }
}

void TodoTable::removeList(const QValueList<int> &t)
{
    if (ro)
	return;
    for (QValueList<int>::ConstIterator it = t.begin(); it != t.end(); ++it) {
	PrTask t;
	t.setUid( TodoXmlIO::uuidFromInt(*it) );
	mTasks->removeTask(t);
    }
    
    reload();
}


void TodoTable::updateEntry(const PimTask &todo )
{
    if (ro)
	return;
    mTasks->updateTask( todo );
    reload();
    setCurrentEntry( todo.uid() );
}

void TodoTable::sortColumn( int , bool, bool )
{
/*
    // external sort.
    switch(col) {
	case 0:
	    ta.setSortOrder(SortedTasks::Completed);
	    break;
	case 1:
	    ta.setSortOrder(SortedTasks::Priority);
	    break;
	case 2:
	    ta.setSortOrder(SortedTasks::Description);
	    break;
	case 3:
	    ta.setSortOrder(SortedTasks::DueDate);
	    break;
    }
    // gets QTable to repaint....
    refresh();
*/
}

void TodoTable::rowHeightChanged( int )
{
}

void TodoTable::fontChange( const QFont &oldFont )
{
    QFont f = font();
    if ( oldFont != f ) {
	QFontMetrics fm(f);
	RowHeight = QMAX(20, fm.height() + 2);

	// keep it an even number, a bit over 3 / 4 of rowHeight.
	BoxSize =  ( RowHeight * 3 ) / 4;
	BoxSize += BoxSize % 2;

	fitHeadersToWidth();
    }
    QTable::fontChange(oldFont);
}

void TodoTable::contentsMousePressEvent( QMouseEvent *e )
{
    if ( mSel == Extended ) {
	selectionBeginRow = -1;
	selectionEndRow = -1;
	bState = e->button();
    }

    QTable::contentsMousePressEvent( e );
}

void TodoTable::contentsMouseReleaseEvent( QMouseEvent *e )
{
    if ( mSel == Extended ) {
	//selectionBeginRow = -1;
	bState = Qt::NoButton;
    }

    QTable::contentsMouseReleaseEvent( e );
}

void TodoTable::resizeEvent( QResizeEvent *e )
{
    QTable::resizeEvent( e );
    
    // we receive a resize event from qtable in the middle of the constrution, since
    // QTable::columnwidth does qApp->processEvents.  Ignore this event as it causes
    // all sorts of init problems for us
}

void TodoTable::showEvent( QShowEvent *e)
{
    QTable::showEvent(e);
#ifdef QTOPIA_DESKTOP
    fitHeadersToWidth();
#endif
}

void TodoTable::fitHeadersToWidth()
{
    int w = width() - frameWidth();
    if (contentsHeight() >= (height() - horizontalHeader()->height()) )
	w -= (style().scrollBarExtent().width());
    
    calcFieldSizes( 0, w );
    refresh();
}

void TodoTable::calcFieldSizes(int oldSize, int size)
{
//    qDebug("resize event called for todotable, which we will process");
    constructorDone = FALSE; //don't let QTable mess up our logic
    int col = headerKeyFields.count();
    
    int max = 0;
    int i;
    for (i = 0; i < col; i++) {
	max += columnWidth(i);
    }
    if ( oldSize < max )
	oldSize = max;
    
    int accumulated = 0;
    for (i = 0; i < col; i++) {
	// not all fields benifit from streatching
	int min = minimumFieldSize( (PimTask::TaskFields) headerKeyFields[i] );
	int newColLen = min;
	switch (headerKeyFields[i]) {
	    case PimTask::CompletedField:
	    case PimTask::PercentCompleted:
	    case PimTask::DueDateYear:
	    case PimTask::DueDateMonth:
	    case PimTask::DueDateDay:
	    case PimTask::Status:
	    case PimTask::StartedDate:
	    case PimTask::CompletedDate:
	    case PimTask::Priority:
		newColLen = min;
		break;
	    default:
		{
		    float l = (float) columnWidth( i )
			/ (float) oldSize;
		    float l2 = l * size;
		    newColLen = (int) l2;
		    if ( newColLen < min )
			newColLen =  min;
		}
		break;
	}
	
	// make sure we fill out the space if there's some integer rounding leftover
	if ( i == col - 1 && size - accumulated - 2 > min )
	   newColLen = size - accumulated - 2;
	else
	    accumulated += newColLen;

	setColumnWidth( i, newColLen );
    }

    constructorDone = TRUE;
}

QString TodoTable::categoryLabel( int id )
{
    // This is called seldom, so calling a load in here
    // should be fine.
    mCat.load( categoryFileName() );
    if ( id == -1 )
	return tr( "Unfiled" );
    else if ( id == -2 )
	return tr( "All" );

    QArray<int> ids( 1 );
    ids[0] = id;
    
    return mCat.displaySingle( "Todo List", ids, Categories::ShowFirst ); //No tr
}

void TodoTable::cornerButtonClicked()
{
    int row = pos( TodoXmlIO::uuidToInt(d->modifiedTask()) );
    if ( row > -1 ) {
	int col = currentColumn();
	setCurrentCell( row, col );
	ensureCellVisible( row, col );

    }
}

void TodoTable::refresh()
{
    mLastRowShown = -1;
    if ( d->hasModifiedTask() ) {
	// we might have an invalid uid at this point.  Check row within bounds
	// just to be sure
	if ( currentRow() < (int)mFiltered.size() && mTasks->filteredItem(currentRow()).uid() == d->modifiedTask() ) {
	    d->clearModifiedTask();
	} else  {
	    // Our modified task can have been filtered out
	    int i = pos( TodoXmlIO::uuidToInt(d->modifiedTask()) );
	    if ( i == -1 )
		d->clearModifiedTask();
	}
    }

    if ( numRows() != (int)mFiltered.size() )
	setNumRows(mFiltered.size());
    else
	updateContents(contentsX(), contentsY(), visibleWidth(), visibleHeight());
}

// find next form current row, OR find
void TodoTable::find( const QString &findString )
{
    if (findString.isEmpty())
	return;
    searchResults = mTasks->filteredSearch(findString);
    //qDebug("search size %d", searchResults.count());
    if (searchResults.count() == 0) {
	emit findNotFound();
    }

#define FASTFIND
#ifdef FASTFIND
    currFind = searchResults.begin();
#else
    for (currFind = searchResults.begin(); currFind != searchResults.end(); ++currFind) {
	if (currentRow() <= pos(*currFind)) {
	    break;
	}
    }
#endif
    clearSelection( TRUE );


    if (currFind == searchResults.end()) {
	emit findWrapAround();
    } else {
	setCurrentCell( pos(*currFind), currentColumn() );
	// we should always be able to wrap around and find this again,
	// so don't give confusing not found message...
	emit findFound();
    }
}

void TodoTable::findNext()
{
    if (searchResults.count() == 0)
	emit findNotFound();

    if (currFind == searchResults.end())
	currFind = searchResults.begin();
    else
	++currFind;

    if (currFind == searchResults.end()) {
	emit findWrapAround();
    } else {
	//qDebug("found pos %d, (%d)", pos(*currFind), *currFind);
	setCurrentCell( pos(*currFind), currentColumn() );
	emit findFound();
    }
}

#if 0 
static bool taskCompare( const PimTask &task, const QRegExp &r, int category )
{
    bool returnMe;
    QArray<int> cats;
    cats = task.categories();

    returnMe = false;
    if ( (category == -1 && cats.count() == 0) || category == -2 )
	returnMe = task.match( r );
    else {
	int i;
	for ( i = 0; i < int(cats.count()); i++ ) {
	    if ( cats[i] == category ) {
		returnMe = task.match( r );
		break;
	    }
	}
    }
    return returnMe;
}
#endif 

int TodoTable::rowHeight( int ) const
{
    return RowHeight;
}

int TodoTable::rowPos( int row ) const
{
    return RowHeight*row;
}

int TodoTable::rowAt( int pos ) const
{
    return QMIN( pos/RowHeight, numRows()-1 );
}

void TodoTable::paintFocus(
#ifndef QTOPIA_PHONE        
        QPainter *p, const QRect &r
#else
        QPainter *, const QRect &
#endif
        )
{
#ifndef QTOPIA_PHONE
    if ( !constructorDone )
	return;
    QRect fr(0, 0, r.width(), r.height() );
    if ( mSel == NoSelection ) {
	QTable::paintFocus(p, r);
    } else {
	p->setPen( QPen( black, 1) );
	p->setBrush( NoBrush );
	p->drawRect( fr.x(), fr.y(), fr.width()-1, fr.height()-1 );
    }
#endif
}
static long n1 = -1;
static long n2 = -1;
void TodoTable::paintCell(QPainter *p, int row, int col,
	const QRect &cr, bool)
{
    if ( !constructorDone )
	return;

#if defined(Q_WS_WIN)
    const QColorGroup &cg = ( style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus ) ? palette().inactive() : colorGroup() );
#else
    const QColorGroup &cg = colorGroup();
#endif

    p->save();

    if (mLastRowShown != row) {
#ifdef Q_OS_UNIX
	struct timeval t1, t2;
	gettimeofday(&t1, 0);
#endif
	mLastTaskShown = mTasks->filteredItem(row);
#ifdef Q_OS_UNIX
	gettimeofday(&t2, 0);
	if (n2 == -1) {
	    n2 = t1.tv_usec;
	    n1 = t2.tv_usec - t1.tv_usec;
	} else {
	    //qDebug("one row, %ld, one get %ld, (prev %ld", t1.tv_usec - n2, t2.tv_usec - t1.tv_usec, n1 );
	    n2 = -1;
	    n1 = -1;
	}
#endif
	mLastRowShown = row;
    }
    PimTask task = mLastTaskShown;
    
    bool selected = FALSE;
    int fromRow = QMIN(selectionBeginRow, selectionEndRow);
    int toRow = QMAX(selectionBeginRow, selectionEndRow);
    if ( mSel != NoSelection && 
	    fromRow != -1 && toRow != -1 &&
	    fromRow <= row && toRow >= row &&
	    fromRow != toRow) // don't 'select' one item
	selected = TRUE;
/*    
    bool current = (row == currentRow() );
    bool focusCell = (row == currentRow() && col == currentColumn());
*/    
    int field = headerKeyFields[ col ];
  
    QBrush backgrnd;
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    if (row % 2 && style().extendedBrush(QStyle::AlternateBase).style() != NoBrush)
	backgrnd = style().extendedBrush(QStyle::AlternateBase);
    else
#endif
	backgrnd = cg.brush( QColorGroup::Base );


    if ( selected 
#ifdef QTOPIA_PHONE
	    || currentRow() == row
	    && (currentColumn() == col || ro)
#endif
	    /*&& !focusCell */ ) {
	p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Highlight ) );
	p->setPen(cg.highlightedText());
    } else if ( d->hasModifiedTask() && task.uid() == d->modifiedTask() ) {
	if ( field == d->editedTaskField() ) {
	    p->fillRect( 0, 0, cr.width(), cr.height(), backgrnd );
	    p->setPen(cg.text() );
	} else {
	    p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Mid ) );
	    p->setPen(cg.light() );
	}
    } else {
	p->fillRect( 0, 0, cr.width(), cr.height(), backgrnd );
	p->setPen(cg.text());
    }

#ifndef QTOPIA_PHONE
    p->drawLine( 0, cr.height() - 1, cr.width() - 1, cr.height() - 1 );
    p->drawLine( cr.width() - 1, 0, cr.width() - 1, cr.height() - 1 );
#endif
    QFont f = p->font();
    QFontMetrics fm(f);

    switch(field) {
	case PimTask::CompletedField:
	    {
		//qDebug("BoxSize, Rowheight, %d, %d", BoxSize, RowHeight);
		// completed field
		int marg = ( cr.width() - BoxSize ) / 2;
		int x = 0;
		int y = ( cr.height() - BoxSize ) / 2;
//		if ( !selected ) {
//		    p->setPen( QPen( cg.highlightedText() ) );
//		} else {
		    p->setPen( QPen( cg.text() ) );
//		}
		
		p->drawRect( x + marg+1, y+1, BoxSize-2, BoxSize-2 );
		p->fillRect( x + marg+2, y+2, BoxSize-4, BoxSize-4, cg.brush( QColorGroup::Base ) );
		
		p->setPen( darkGreen );
		x += 1;
		y += 1;
		if ( task.isCompleted() ) {
		    int i, xx, yy;
		    int sseg = BoxSize / 4;
		    int lseg = BoxSize / 2;
		    sseg -=2; // to fit in BoxSize.
		    lseg -=1;
		    xx = x+sseg+marg;
		    yy = y + lseg;
		    QPointArray a( 6*2 );
		    // tripple thickens line.
		    for (i=0; i < 3; i++) {
			a.setPoint(4*i, xx, yy);
			a.setPoint(4*i+1, xx+sseg, yy+sseg);
			a.setPoint(4*i+2, xx+sseg, yy+sseg);
			a.setPoint(4*i+3, xx+sseg+lseg, yy+sseg-lseg);
			yy++;
		    }
		    p->drawLineSegments( a );
		}
	    }
	    break;
	case PimTask::Priority:
	    // priority field
	    {
		QString text = QString::number(task.priority());
		p->drawText(2,2 + fm.ascent(), text);
	    }
	    break;
	case PimTask::Description:
	    // description field
	    {
		p->drawText(2,2 + fm.ascent(), task.description() );
	    }
	    break;
	case PimTask::Notes:
		//must remove any crlf from the text
		p->drawText(2,2 + fm.ascent(), task.notes().simplifyWhiteSpace() ); 
		break;
	case PimTask::StartedDate:
	    {
		if ( task.hasStartedDate() )
		    p->drawText(2,2 + fm.ascent(), TimeString::localYMD( task.startedDate() ) );
		else
		    p->drawText(2,2 + fm.ascent(), tr("Not started") );
	    }
	    break;
	case PimTask::CompletedDate:
	    {
		if ( task.isCompleted() )
		    p->drawText(2,2 + fm.ascent(), TimeString::localYMD( task.completedDate() ) );
		else
		    p->drawText(2,2 + fm.ascent(), tr("Unfinished") );
	    }
	    break;

	case PimTask::PercentCompleted:
	    {
		p->drawText(2,2 + fm.ascent(), QString::number( task.percentCompleted() ) + "%" );
	    }
	    break;
	case PimTask::Status:
	{
		p->drawText(2,2 + fm.ascent(), statusToText( task.status() ));
	}
	break;
	/*
	case PimTask:::
	    {
		QString text;
		if (task.hasDueDate()) {
		    text = "HAS";
		} else {
		    text = tr("None");
		}
		p->drawText(2,2 + fm.ascent(), text);
	    }
	    break;
	*/
    }
    p->restore();
}

void TodoTable::priorityChanged(int)
{}

void TodoTable::delayCancelEdit()
{
    QTimer::singleShot(0, this, SLOT(cancelEdit()));
}

void TodoTable::delaySetCellContentFromEditor()
{
    QTimer::singleShot(0, this, SLOT(setCellContentFromEditor()));
}

QWidget *TodoTable::createEditor(int , int , bool ) const
{
    return 0;
}

void TodoTable::cancelEdit()
{
#ifdef QTOPIA_PHONE
    setFocus();
    d->clearModifiedTask();
    le->hide();
#endif
}

void TodoTable::setCellContentFromEditor()
{
    int res = le->currentItem() + 1;
    PimTask task = mTasks->filteredItem(currentRow());
    if (task.priority() != res) {
	task.setPriority( (PimTask::PriorityValue) res);
	d->setModifiedTask( task.uid(), PimTask::Priority);
	emit updateTask( task );
    } else {
	setFocus();
	d->clearModifiedTask();
    }
    le->hide();
    /*
       Old way of working... doesn't seem to be a good idea anymore?

    setCellContentFromEditor( currentRow(), currentColumn() );
    clearCellWidget( currentRow(), currentColumn() );
    */
}

void TodoTable::setCellContentFromEditor(int row, int col)
{
    QWidget *w = cellWidget(row,col);

    PimTask task = mTasks->filteredItem(row);
    if (w->inherits("QComboBox") ) {
	int res = ((QComboBox *)w)->currentItem() + 1;
	if (task.priority() != res) {
	    int i = ((QComboBox *)w)->currentItem() + 1;
	    task.setPriority( (PimTask::PriorityValue) i);
	    d->setModifiedTask( task.uid(), PimTask::Priority);
	    emit updateTask( task );
	    //refresh();
	} else {
	    setFocus();
	    d->clearModifiedTask();
	}
    }
}

void TodoTable::clearCellWidget(int row, int col)
{
    QTable::clearCellWidget(row, col);
}

void TodoTable::readSettings()
{
    QStringList selectedFields, sizeList;

    {
#ifdef QTOPIA_DESKTOP
	QSettings *settings = gQtopiaDesktopConfig->createQSettings();

	selectedFields = settings->readListEntry("/todolist/fields" );
	sizeList = settings->readListEntry("/todolist/colwidths" );

	mSortColumn = settings->readNumEntry("/todolist/sortcolumn", 0);
	ascSort = settings->readBoolEntry("/todolist/ascsort", FALSE);

	gQtopiaDesktopConfig->deleteQSettings();
#else
	Config config( "todo" );
	config.setGroup( "View" );

	selectedFields = config.readListEntry("fields", ',');
	sizeList = config.readListEntry("colwidths",',');
	mSortColumn = config.readNumEntry("sortcolumn", 0 );
	ascSort = config.readBoolEntry("ascsort", 0 );
#endif
    }

    if ( !selectedFields.count() ) {
	setFields( defaultFields() );
    } else {
	QMap<QCString, int> identifierToKey = PimTask::identifierToKeyMap();
	for ( QStringList::Iterator it = selectedFields.begin(); it != selectedFields.end(); ++it) {
	    int field = identifierToKey[ (*it).data() ];
	    headerKeyFields.append( field );
	}

	setFields( headerKeyFields, sizeList );
    }

    // whether sort changes or not, readConfig is expected to reload the data.
    reload(); 
#ifndef QTOPIA_PHONE
    if ( mSortColumn > -1 ) {
#ifndef Q_OS_WIN32
	horizontalHeader()->setSortIndicator(mSortColumn,!ascSort);
#else
	horizontalHeader()->setSortIndicator(mSortColumn, ascSort);
#endif
    }
#endif
    constructorDone = TRUE;
    fitHeadersToWidth();
    refresh();
}

void TodoTable::saveSettings()
{
    // don't write settings, could break things.
    if (ro)
	return;

    QMap<int,QCString> keyToIdentifier = PimTask::keyToIdentifierMap();
    QHeader *header = horizontalHeader();
    QStringList fieldList, sizeList;
    for ( int i = 0; i < header->count(); i++) {
	fieldList.append( keyToIdentifier[ headerKeyFields[i]  ]  );
	sizeList.append( QString::number(header->sectionSize(i)) );
    }

#ifdef QTOPIA_DESKTOP
    QSettings *settings = gQtopiaDesktopConfig->createQSettings();

    settings->writeEntry( "/todolist/fields", fieldList );
    settings->writeEntry( "/todolist/colwidths", sizeList );
    settings->writeEntry( "/todolist/sortcolumn", mSortColumn );
    settings->writeEntry( "/todolist/ascsort", ascSort );

    gQtopiaDesktopConfig->deleteQSettings();
#else
    Config config( "todo" );
    config.setGroup( "View" );
    config.writeEntry("fields", fieldList, ',' );
    config.writeEntry("colwidths", sizeList, ',' );
    config.writeEntry("sortcolumn", mSortColumn );
    config.writeEntry("ascsort", ascSort );
#endif

}

void TodoTable::setFields(QValueList<int> f)
{
    QHeader *header = horizontalHeader();
    QStringList sizes;

    if ( f.count() == 0 ) {
	f = defaultFields();
	headerKeyFields.clear();
    }

    // map old sizes to new pos in header list
    for ( int i = 0; i < (int) f.count(); i++) {
	int pos = headerKeyFields.findIndex( *f.at(i) );
	//qDebug(" f key %d found at %d", *f.at(i), pos );
	if ( pos > -1 && pos < header->count() )
	    sizes.append( QString::number( header->sectionSize(pos) ) );
	else
	    sizes.append( QString::number( defaultFieldSize( (PimTask::TaskFields) *f.at(i) ) ));
    }
    
    setFields(f, sizes);

    if ( isVisible() )
	fitHeadersToWidth();
}

void TodoTable::setFields(QValueList<int> f, QStringList sizes)
{
    QHeader *header = horizontalHeader();
    int prevSortKey = -1;

    if ( mSortColumn > -1 && headerKeyFields.count() )
	prevSortKey = headerKeyFields[mSortColumn];

    headerKeyFields = f;

    while ( header->count() )
	header->removeLabel( header->count() - 1 );
    
    // We have to create the internal list before calling QTable::setNumCols as
    // setnNumCols forces a repaint
    QValueList<int>::ConstIterator iit;
    int i = 0;
    for (iit = f.begin(); iit != f.end(); ++iit) {
	if ( *iit == prevSortKey ) {
	    mSortColumn = i;
	    break;
	}
	i++;
    }
    setNumCols( f.count() );
    
    QMap<int, QString> trFields = PimTask::trFieldsMap();
    i = 0;
    for (iit = f.begin(); iit != f.end(); ++iit) {
	if ( *iit == PimTask::CompletedField )
	    header->setLabel( i++, Resource::loadPixmap("task-completed"), "" );
	else if ( *iit == PimTask::Priority )
	    header->setLabel( i++, Resource::loadPixmap("task-priority"), "" );
	else
	    header->setLabel(i++, trFields[*iit] );
    }

    i = 0;
    for (QStringList::ConstIterator it = sizes.begin(); it != sizes.end(); ++it) {
	if ( i < (int) f.count() )
	    setColumnWidth(i, (*it).toInt() );
	
	i++;
    }
    horizontalHeader()->show();
}

QValueList<int> TodoTable::fields()
{
    return headerKeyFields;
}

QValueList<int> TodoTable::defaultFields()
{
    QValueList<int> l;
    l.append( PimTask::CompletedField );
    l.append( PimTask::Priority );
    l.append( PimTask::Description );

    return l;
}

int TodoTable::defaultFieldSize(PimTask::TaskFields f)
{
    switch( f ) {
	case PimTask::CompletedField: return BoxSize + 6;
	case PimTask::Status: return 70;
	case PimTask::Description: return 157;
	case PimTask::Priority:
	   {
	       QFont fn = font();
	       QFontMetrics fm(fn);
	       return fm.width(" 8 "); // no tr.. used for size to print number
	   }
	case PimTask::PercentCompleted: return 45;
	case PimTask::StartedDate: return 100;
	case PimTask::CompletedDate: return 100;
	default: return 70;
    }
}

int TodoTable::minimumFieldSize(PimTask::TaskFields f)
{
    return QMIN(defaultFieldSize(f), 45);
}

void TodoTable::headerClicked(int 
#ifndef QTOPIA_PHONE
        h
#endif
)
{
#ifndef QTOPIA_PHONE
    if ( h != mSortColumn ) {
	mSortColumn = h;
	ascSort = FALSE;
    } else
	ascSort = !ascSort;

#ifndef Q_OS_WIN32
    horizontalHeader()->setSortIndicator(mSortColumn,!ascSort);
#else
    horizontalHeader()->setSortIndicator(mSortColumn, ascSort);
#endif
    reload();
#endif
}

QString TodoTable::statusToText(PimTask::TaskStatus s)
{
    switch( s ) {
	default: return tr("Not Started");
	case PimTask::InProgress: return tr("In Progress");
	case PimTask::Completed: return tr("Completed");
	case PimTask::Waiting: return tr("Waiting");
	case PimTask::Deferred:  return tr("Deferred");
    }
}

#include "todotable.moc"
