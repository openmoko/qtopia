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

#include "todotable.h"

#include <qtopia/categoryselect.h>
#include <qtopia/xmlreader.h>
#include <qtopia/resource.h>

#include <qasciidict.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qpainter.h>
#include <qtextcodec.h>
#include <qtimer.h>
#include <qdatetime.h>

#include <qcursor.h>
#include <qregexp.h>

#include <errno.h>
#include <stdlib.h>


static const int BoxSize = 14;
static const int RowHeight = 20;

static bool taskCompare( const PimTask &task, const QRegExp &r, int category );

TodoTable::TodoTable( QWidget *parent, const char *name )
    : QTable( 0, 3, parent, name ),
      ta(TaskIO::ReadWrite),
      mCat( 0 ),
      currFindRow( -2 ) 
{
    mCat.load( categoryFileName() );
    setSorting( TRUE );
    setSelectionMode( NoSelection );
    setColumnStretchable( 2, TRUE );
    setColumnWidth( 0, 21 );
    setColumnWidth( 1, 35 );
    setLeftMargin( 0 );
    setFrameStyle( NoFrame );
    verticalHeader()->hide();
    horizontalHeader()->setLabel( 0, Resource::loadPixmap("task-completed"), "" );
    horizontalHeader()->setLabel( 1, tr( "Prior." ) );
    horizontalHeader()->setLabel( 2, tr( "Description" ) );
    connect( this, SIGNAL( clicked( int, int, int, const QPoint & ) ),
	     this, SLOT( slotClicked( int, int, int, const QPoint & ) ) );
    connect( this, SIGNAL( pressed( int, int, int, const QPoint & ) ),
	     this, SLOT( slotPressed( int, int, int, const QPoint & ) ) );
    connect( this, SIGNAL( currentChanged( int, int ) ),
             this, SLOT( slotCurrentChanged( int, int ) ) );

    //connect( &ta, SIGNAL( todolistUpdated() ), this, SLOT(refresh()));

    menuTimer = new QTimer( this );
    connect( menuTimer, SIGNAL(timeout()), this, SLOT(slotShowMenu()) );
}

void TodoTable::addEntry( const PimTask &todo )
{
    ta.addTask(todo);
    refresh();
}

void TodoTable::updateEntry( const PimTask &todo )
{
    ta.updateTask(todo);
    refresh();
}

void TodoTable::removeEntry( const PimTask &todo )
{
    ta.removeTask(todo);
    refresh();
}

void TodoTable::flush()
{
    ta.saveData();
}

void TodoTable::reload()
{
    ta.ensureDataCurrent(TRUE);
    refresh();
}

void TodoTable::slotClicked( int row, int col, int , const QPoint &pos )
{
    if ( !cellGeometry( row, col ).contains(pos) )
	return;
    if (row < 0 || row >= (int)ta.sortedTasks().count())
	return;

    PimTask task(*ta.sortedTasks()[row]);
    // let's switch on the column number...
    switch ( col )
    {
        case 0: 
	    {
		int x = pos.x() - columnPos( col );
		int y = pos.y() - rowPos( row );
		int w = columnWidth( col );
		int h = rowHeight( row );
		if ( x >= ( w - BoxSize ) / 2 && x <= ( w - BoxSize ) / 2 + BoxSize &&
			y >= ( h - BoxSize ) / 2 && y <= ( h - BoxSize ) / 2 + BoxSize ) {
		    task.setCompleted(!task.isCompleted());
		    ta.updateTask(task);
		    refresh();
		}
	    }
            break;
        case 1:
	    // fake a double click.
	    {
		QKeyEvent e(QEvent::KeyPress, 0x20, 0, Key_Space, " ");
		QTable::keyPressEvent(&e);
	    }
            break;
        case 2:
            // may as well edit it...
	    menuTimer->stop();
//            emit signalEdit();
            break;
    }
}

void TodoTable::slotPressed( int row, int col, int, const QPoint &pos )
{
    if ( col == 2 && cellGeometry( row, col ).contains(pos) )
	menuTimer->start( 750, TRUE );
}

void TodoTable::slotShowMenu()
{
    emit signalShowMenu( QCursor::pos() );
}

void TodoTable::slotCurrentChanged( int, int )
{
    menuTimer->stop();
}

PimTask TodoTable::currentEntry()
{
    if (ta.sortedTasks().count() == 0)
	return PimTask();
    if (currentRow() >= (int)ta.sortedTasks().count()) {
	setCurrentCell(ta.sortedTasks().count() - 1, currentColumn());
    } else if (currentRow() < 0) {
	setCurrentCell(0, currentColumn());
    }
    return PimTask(*(ta.sortedTasks()[currentRow()]));
}

void TodoTable::setCurrentEntry(QUuid &u)
{
    int rows, row;
    rows = numRows();

    for ( row = 0; row < rows; row++ ) {
	if ( ta.sortedTasks().at(row)->uid() == u) {
	    setCurrentCell(row, currentColumn());
	    break;
	}
    }
}

void TodoTable::removeCurrentEntry()
{
    ta.removeTask(currentEntry());
    refresh();
}

void TodoTable::sortColumn( int col, bool /*ascending*/, bool /*wholeRows*/ )
{
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
}

void TodoTable::rowHeightChanged( int )
{
}

void TodoTable::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Space || e->key() == Key_Return ) {
	switch ( currentColumn() ) {
	    case 0: 
		{
		    PimTask task(*(ta.sortedTasks()[currentRow()]));
		    task.setCompleted(!task.isCompleted());
		    ta.updateTask(task);
		    refresh();
		}
		break;
	    case 1:
		//Opens the widget
		QTable::keyPressEvent( e );
		break;
	    case 2:
		emit signalEdit();
	    default:
		break;
	}
    } else {
	QTable::keyPressEvent( e );
    }
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
    return mCat.label( "Todo List", id );
}

void TodoTable::refresh()
{
    setNumRows(ta.sortedTasks().count());
}

void TodoTable::slotDoFind( const QString &findString, int category )
{
    // we have to iterate through the table, this gives the illusion that
    // sorting is actually being used.
    static bool wrapAround = false;
    if ( currFindString != findString ) {
	currFindRow = -1;
	wrapAround = false;
    }
    currFindString = findString;
    if ( currFindRow < -1 )
	currFindRow = currentRow() - 1;
    clearSelection( TRUE );
    QRegExp r( findString );

    r.setCaseSensitive( FALSE );
    int rows = numRows();
    int row;
    for ( row = currFindRow + 1; row < rows; row++ ) {
	if ( taskCompare( *(ta.sortedTasks()[row]), r, category) )
	    break;
    }
    if ( row >= rows ) {
	currFindRow = -1;
	if ( wrapAround )
	    emit findWrapAround();
	else
	    emit findNotFound();
    } else {
	currFindRow = row;
	QTableSelection foundSelection;
	foundSelection.init( currFindRow, 0 );
	foundSelection.expandTo( currFindRow, numCols() - 1 );
	addSelection( foundSelection );
	setCurrentCell( currFindRow, numCols() - 1 );
	// we should always be able to wrap around and find this again,
	// so don't give confusing not found message...
	emit findFound();
	wrapAround = true;
    }
}

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

void TodoTable::paintCell(QPainter *p, int row, int col,
	const QRect &cr, bool) 
{
#if defined(Q_WS_WIN)
    const QColorGroup &cg = ( !drawActiveSelection && style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus ) ? palette().inactive() : colorGroup() );
#else
    const QColorGroup &cg = colorGroup();
#endif

    p->save();

    PimTask task(*(ta.sortedTasks()[row]));
    p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Base ) );
    QPen op = p->pen();
    p->setPen(cg.mid());
    p->drawLine( 0, cr.height() - 1, cr.width() - 1, cr.height() - 1 );
    p->drawLine( cr.width() - 1, 0, cr.width() - 1, cr.height() - 1 );
    p->setPen(op);

    QFont f = p->font();
    QFontMetrics fm(f);

    switch(col) {
	case 0: 
	    {
		// completed field
		int marg = ( cr.width() - BoxSize ) / 2;
		int x = 0;
		int y = ( cr.height() - BoxSize ) / 2;
		p->setPen( QPen( cg.text() ) );
		p->drawRect( x + marg, y, BoxSize, BoxSize );
		p->drawRect( x + marg+1, y+1, BoxSize-2, BoxSize-2 );
		p->setPen( darkGreen );
		x += 1;
		y += 1;
		if ( task.isCompleted() ) {
		    QPointArray a( 9*2 );
		    int i, xx, yy;
		    xx = x+2+marg;
		    yy = y+4;
		    for ( i=0; i<4; i++ ) {
			a.setPoint( 2*i,   xx, yy );
			a.setPoint( 2*i+1, xx, yy+2 );
			xx++; yy++;
		    }
		    yy -= 2;
		    for ( i=4; i<9; i++ ) {
			a.setPoint( 2*i,   xx, yy );
			a.setPoint( 2*i+1, xx, yy+2 );
			xx++; yy--;
		    }
		    p->drawLineSegments( a );
		}
	    }
	    break;
	case 1:
	    // priority field
	    {
		QString text = QString::number(task.priority());
		p->drawText(2,2 + fm.ascent(), text);
	    }
	    break;
	case 2:
	    // description field
	    {
		QString text = task.description();
		p->drawText(2,2 + fm.ascent(), text);
	    }
	    break;
	case 3:
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
    }
    p->restore();
}

void TodoTable::setShowCompleted(bool b)
{
    ta.setCompletedFilter(!b);
    refresh();
}

bool TodoTable::showCompleted() const
{
    return !ta.completedFilter();
}

int TodoTable::showCategory() const 
{
    return ta.filter();
}

void TodoTable::setShowCategory(int c)
{
    ta.setFilter(c);
    refresh();
}

/*  Need to store changes in priority as the user selects them.  Otherwise they
    might be lost in a closeevent
*/
void TodoTable::priorityChanged(int)
{
    QTimer::singleShot(0, this, SLOT(setCellContentFromEditor()));
}

QWidget *TodoTable::createEditor(int row, int col, bool ) const
{
    switch (col) {
	case 0:
	    return 0;
	case 1:
	    {
		QComboBox *cb = new QComboBox( viewport() );
		cb->insertItem( "1" );
		cb->insertItem( "2" );
		cb->insertItem( "3" );
		cb->insertItem( "4" );
		cb->insertItem( "5" );
		cb->setCurrentItem( ta.sortedTasks()[row]->priority() - 1 );
		
		connect( cb, SIGNAL(activated(int)), this, SLOT(priorityChanged(int)) );
		return cb;
	    }
	default:
	    return 0;
    }
}

void TodoTable::setCellContentFromEditor()
{
    setCellContentFromEditor( currentRow(), currentColumn() );
    clearCellWidget( currentRow(), currentColumn() );
}

void TodoTable::setCellContentFromEditor(int row, int col)
{
    QWidget *w = cellWidget(row,col);
    
    PimTask task(*(ta.sortedTasks()[row]));
    if (w->inherits("QComboBox") && col == 1) {
	int res = ((QComboBox *)w)->currentItem() + 1;
	if (task.priority() != res) {
	    task.setPriority( ((QComboBox *)w)->currentItem() + 1);
	    ta.updateTask(task);
	    refresh();
	}
    }
}
