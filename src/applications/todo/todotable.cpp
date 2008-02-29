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
#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/timestring.h>

#include <qasciidict.h>
#include <qcombobox.h>
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
#include <qsettings.h>
#endif

#include <errno.h>
#include <stdlib.h>


static int BoxSize = 14;
static int RowHeight = 20;

static Qt::ButtonState bState = Qt::NoButton;
static int selectionBeginRow = -1;

static QString applicationPath;
static bool constructorDone = FALSE;

static bool taskCompare( const PimTask &task, const QRegExp &r, int category );

/* XPM */
static char * menu_xpm[] = {
"12 12 5 1",
" 	c None",
".	c #000000",
"+	c #FFFDAD",
"@	c #FFFF00",
"#	c #E5E100",
"            ",
"            ",
"  ......... ",
"  .+++++++. ",
"  .+@@@@#.  ",
"  .+@@@#.   ",
"  .+@@#.    ",
"  .+@#.     ",
"  .+#.      ",
"  .+.       ",
"  ..        ",
"            "};

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
	    scrollButton->setPixmap( QPixmap( (const char**)menu_xpm) );
	    scrollButton->hide();
	}

	return scrollButton;
    }

    void setModifiedTask(QUuid id, int field)
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

TodoTable::TodoTable(TodoXmlIO *tasks, QWidget *parent, const char *name, const char *appPath )
    : QTable( 0, 0, parent, name ),
      mCat( 0 ),
      currFindRow( -2 )
{
    applicationPath = appPath;

    QFont f = font();
    QFontMetrics fm(f);
    RowHeight = QMAX(20, fm.height() + 2);

    // keep it an even number, a bit over 3 / 4 of rowHeight.
    BoxSize =  ( RowHeight * 3 ) / 4;
    BoxSize += BoxSize % 2;
    
    // do not!! put the below assignment in the constructor init, as the todoplugin for
    // qtopiadesktop fails to resolve the symbol
    mTasks = tasks;
    mSel = NoSelection;

    d = new TablePrivate();

    setCornerWidget( d->cornerButton(this) );
    cornerWidget()->hide();
    connect( d->cornerButton(this), SIGNAL( clicked() ),
	    this, SLOT( cornerButtonClicked() ) );

    mCat.load( categoryFileName() );
    setSorting( TRUE );
    QTable::setSelectionMode( QTable::NoSelection );

    setLeftMargin( 0 );
#ifndef QTOPIA_DESKTOP
    setFrameStyle( NoFrame );
#endif
    verticalHeader()->hide();
    horizontalHeader()->hide();

    connect(horizontalHeader(), SIGNAL(clicked(int)), this, SLOT(headerClicked(int)) );
    mSortColumn = -1;
    ascSort = FALSE;

    connect( this, SIGNAL( clicked( int, int, int, const QPoint & ) ),
	     this, SLOT( slotClicked( int, int, int, const QPoint & ) ) );
    connect( this, SIGNAL( pressed( int, int, int, const QPoint & ) ),
	     this, SLOT( slotPressed( int, int, int, const QPoint & ) ) );
    connect( this, SIGNAL( doubleClicked( int, int, int, const QPoint & ) ),
	     this, SLOT( slotDoubleClicked( int, int, int, const QPoint & ) ) );
    connect( this, SIGNAL( currentChanged( int, int ) ),
             this, SLOT( slotCurrentChanged( int, int ) ) );

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
	mSelected.clear();
	refresh();
    }

    mSel = m;
}

QValueList<QUuid> TodoTable::selectedTasks()
{
    QValueList<QUuid> list;
    if ( mSel == Single ) {
    	if ( hasCurrentEntry() )
	    list.append( currentEntry().uid() );
    } else if ( mSel == Extended ) {
	list = mSelected;
	
	// set current entry as selected when none is selected
	if ( !list.count() && hasCurrentEntry() )
	    list.append( currentEntry().uid() );
    }

    return list;
}

QValueList<PimTask> TodoTable::selected()
{
    QValueList<PimTask> list;
    if ( mSel == Single ) {
    	if ( hasCurrentEntry() )
	    list.append( currentEntry() );
    } else if ( mSel == Extended ) {
	for ( QValueList<QUuid>::Iterator it = mSelected.begin(); it != mSelected.end(); ++it) {
	    int i = pos( *it );
	    if ( i > -1 )
		list.append( *mTasks->sortedTasks().at(i) );
	}
	
	// set current entry as selected when none is selected
	if ( !list.count() && hasCurrentEntry() )
	    list.append( currentEntry() );
    }

    return list;

}

void TodoTable::selectAll()
{
    if ( mSel == NoSelection || mSel == Single )
	return;

    selectionBeginRow = -1;
    mSelected.clear();
    for ( uint u = 0; u < mTasks->sortedTasks().count(); u++ ) {
	mSelected.append( mTasks->sortedTasks().at(u)->uid() );
    }
    refresh();
}

// rework to support new selection mode
void TodoTable::setSelection(int /* row */)
{
/*
    if ( mSel != NoSelection ) {
	PimTask task(*mTasks[row]);

	if ( mSel == Extended ) {
	    QValueList<QUuid>::Iterator it = mSelected.find( task.uid() );

	    if ( (bState & Qt::ControlButton) || (bState & Qt::ShiftButton) ) {
		if ( it == mSelected.end() )
		    mSelected.append( task.uid() );
		else
		    mSelected.remove( task.uid() );
	    } else {
		mSelected.clear();
		mSelected.append( task.uid() );
	    }
	    bState = Qt::NoButton;
	} else if ( mSel == Single ) {
	    mSelected.clear();
	    mSelected.append( task.uid() );
	}

	if ( mSelected.count() == 0 )
	    emit currentChanged();
    }
*/
}

// We clear the selection and loop through all the rows since currentChanged
// will skip some rows if you move the pointing device fast enough
void TodoTable::setSelection(int fromRow, int toRow)
{
    // fromLow must be lower
    if ( toRow < fromRow ) {
	int t = toRow;
	toRow = fromRow;
	fromRow = t;
    }
    
    int row = fromRow;
    mSelected.clear();
    while ( row <= toRow ) {
	mSelected.append( mTasks->sortedTasks().at(row)->uid() );
	row++;
    }
}

void TodoTable::reload()
{
    // If the user applies outside filters we have to clear our internal state
    if ( d->editorWidget() ) {
	endEdit(d->editorRow(), d->editorColumn(), FALSE, TRUE );
	d->clearEditorWidget();
    }

    mSelected.clear();

    if ( mSortColumn > -1 ) {
	mTasks->setSorting( headerKeyFields[ mSortColumn ] , ascSort );
//	mTasks->sort();
    }

    refresh();
    setFocus();	// in case of inline widgets grabbing focus
}

int TodoTable::pos(const QUuid &id)
{
    for ( uint u = 0; u < mTasks->sortedTasks().count(); u++ ) {
	if ( mTasks->sortedTasks().at(u)->uid() == id )
	    return u;
    }

    return -1;
}

void TodoTable::slotClicked( int row, int col, int , const QPoint &pos )
{
    if ( d->editorWidget() ) {
	endEdit(d->editorRow(), d->editorColumn(), FALSE, TRUE );
	d->clearEditorWidget();
    }

    if ( !cellGeometry( row, col ).contains(pos) )
	return;
    if (row < 0 || row >= (int)mTasks->sortedTasks().count())
	return;

    PimTask task(*mTasks->sortedTasks().at(row));

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
		    task.setCompleted(!task.isCompleted());
		    task.setCompletedDate( QDate::currentDate() );
		    d->setModifiedTask( task.uid(), PimTask::CompletedField);
		    emit updateTask( task );
		    return;
		}
	    }
            break;
        case PimTask::Priority:
	    {
		QWidget *w = beginEdit(row, col, FALSE);
		d->setEditorWidget(w, row, col );
		QKeyEvent e(QEvent::KeyPress, 0x20, 0, Key_Space, " ");
		QPEApplication::sendEvent(w, &e);
		return;
	    }
            break;
        default:
	    menuTimer->stop();
	    emit clicked();
            break;
    }
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

    if (row < 0 || row >= (int)mTasks->sortedTasks().count())
	return;

    if ( e->key() == Key_Space || e->key() == Key_Return ) {
	int field = headerKeyFields[ col ];
	switch ( field ) {
	    case PimTask::CompletedField:
		{
		    PimTask task(*(mTasks->sortedTasks().at(row)));
		    task.setCompleted(!task.isCompleted());
		    task.setCompletedDate( QDate::currentDate() );
		    d->setModifiedTask( task.uid(), PimTask::CompletedField);
		    emit updateTask( task );
		    return;
		}
		break;
	    case PimTask::Priority:
		{
		    QWidget *w = beginEdit(row, col, FALSE);
		    d->setEditorWidget(w, row, col );
		    QKeyEvent ek(QEvent::KeyPress, 0x20, 0, Key_Space, " ");
		    QPEApplication::sendEvent(w, &ek);
		    return;
		}
		break;
	    default:
		emit clicked();
	    break;
	}
    } else {
	QTable::keyPressEvent( e );
    }
}

void TodoTable::slotPressed( int row, int col, int, const QPoint &pos )
{
    if ( col == 2 && cellGeometry( row, col ).contains(pos) )
	menuTimer->start( 750, TRUE );
}

void TodoTable::slotDoubleClicked(int, int, int, const QPoint &)
{
    emit doubleClicked();
}

void TodoTable::slotCurrentChanged( int row, int )
{
//    qDebug("slotCurrentChanged %d", row );
    bool needRefresh = d->hasModifiedTask();

    if ( mSel == Extended ) {
	if ( (bState & Qt::LeftButton) ) {
	    if ( selectionBeginRow == -1 )
		selectionBeginRow = row;
	    else 
		setSelection( selectionBeginRow, row);

	    needRefresh = TRUE;
	} else {
	    mSelected.clear();
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
    return mTasks->sortedTasks().count() != 0;
}

PimTask TodoTable::currentEntry()
{
    if (mTasks->sortedTasks().count() == 0)
	return PimTask();
    if (currentRow() >= (int)mTasks->sortedTasks().count()) {
	setCurrentCell(mTasks->sortedTasks().count() - 1, currentColumn());
    } else if (currentRow() < 0) {
	setCurrentCell(0, currentColumn());
    }
    return PimTask(*(mTasks->sortedTasks().at(currentRow())));
}

void TodoTable::setCurrentEntry(const QUuid &u)
{
    int rows, row;
    rows = numRows();

    for ( row = 0; row < rows; row++ ) {
	if ( mTasks->sortedTasks().at(row)->uid() == u) {
	    setCurrentCell(row, currentColumn());
	    break;
	}
    }
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
    QFontMetrics fm(f);
    RowHeight = QMAX(20, fm.height() + 2);

    // keep it an even number, a bit over 3 / 4 of rowHeight.
    BoxSize =  ( RowHeight * 3 ) / 4;
    BoxSize += BoxSize % 2;

    QTable::fontChange(oldFont);
}

void TodoTable::contentsMousePressEvent( QMouseEvent *e )
{
    if ( mSel == Extended ) {
	mSelected.clear();
	bState = e->button();
    }

    QTable::contentsMousePressEvent( e );
}

void TodoTable::contentsMouseReleaseEvent( QMouseEvent *e )
{
    if ( mSel == Extended ) {
	selectionBeginRow = -1;
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
	float l = (float) columnWidth( i ) / (float) oldSize;
	float l2 = l * size;
	int newColLen = (int) l2;
	
	int min = minimumFieldSize( (PimTask::TaskFields) headerKeyFields[i] );
	if ( newColLen < min )
	    newColLen =  min;

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
    return mCat.label( "Todo List", id ); // No tr
}

void TodoTable::cornerButtonClicked()
{
    int row = pos( d->modifiedTask() );
    if ( row > -1 ) {
	int col = currentColumn();
	setCurrentCell( row, col );
	ensureCellVisible( row, col );

    }
}

void TodoTable::refresh()
{
    if ( d->hasModifiedTask() ) {
	// we might have an invalid uid at this point.  Check row within bounds
	// just to be sure
	if ( currentRow() < (int) mTasks->sortedTasks().count() && mTasks->sortedTasks().at(currentRow())->uid() == d->modifiedTask() ) {
	    d->clearModifiedTask();
	} else  {
	    // Our modified task can have been filtered out
	    int i = pos( d->modifiedTask() );
	    if ( i == -1 )
		d->clearModifiedTask();
	}
    }

    setNumRows(mTasks->sortedTasks().count());
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
	if ( taskCompare( *(mTasks->sortedTasks().at(row)), r, category) )
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
	setCurrentCell( currFindRow, currentColumn() );
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

void TodoTable::paintFocus(QPainter *p, const QRect &r)
{
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
}

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

    PimTask task(*(mTasks->sortedTasks().at(row)));
    
    bool selected = FALSE;
    if ( mSel != NoSelection  && mSelected.find(task.uid()) != mSelected.end() )
	selected = TRUE;
/*    
    bool current = (row == currentRow() );
    bool focusCell = (row == currentRow() && col == currentColumn());
*/    
    int field = headerKeyFields[ col ];
    
    if ( selected /*&& !focusCell */ ) {
	p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Highlight ) );
	p->setPen(cg.highlightedText());
    } else if ( d->hasModifiedTask() && task.uid() == d->modifiedTask() ) {
	if ( field == d->editedTaskField() ) {
	    p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Base ) );
	    p->setPen(cg.text() );
	} else {
	    p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Mid ) );
	    p->setPen(cg.light() );
	}
    } else {
	p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Base ) );
	p->setPen(cg.text());
    }

    p->drawLine( 0, cr.height() - 1, cr.width() - 1, cr.height() - 1 );
    p->drawLine( cr.width() - 1, 0, cr.width() - 1, cr.height() - 1 );
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
		
		p->drawRect( x + marg, y, BoxSize, BoxSize );
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


/*  Need to store changes in priority as the user selects them.  Otherwise they
    might be lost in a closeevent
*/
void TodoTable::priorityChanged(int)
{
    QTimer::singleShot(0, this, SLOT(setCellContentFromEditor()));
}

QWidget *TodoTable::createEditor(int row, int col, bool ) const
{
    int field = headerKeyFields[ col ];
    switch (field) {
	case PimTask::Priority:
	    {
		QComboBox *cb = new QComboBox( viewport() );
		cb->insertItem( "1" );
		cb->insertItem( "2" );
		cb->insertItem( "3" );
		cb->insertItem( "4" );
		cb->insertItem( "5" );
		cb->setCurrentItem( mTasks->sortedTasks().at(row)->priority() - 1 );

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

    PimTask task(*(mTasks->sortedTasks().at(row)));
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
	QSettings settings;
	settings.insertSearchPath( QSettings::Unix, applicationPath );
	settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

	selectedFields = settings.readListEntry("/palmtopcenter/todolist/fields" );
	sizeList = settings.readListEntry("/palmtopcenter/todolist/colwidths" );

	mSortColumn = settings.readNumEntry("/palmtopcenter/todolist/sortcolumn", 0);
	ascSort = settings.readBoolEntry("/palmtopcenter/todolist/ascsort", FALSE);
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

    if ( mSortColumn > -1 ) {
	reload(); 
#ifndef Q_OS_WIN32
	horizontalHeader()->setSortIndicator(mSortColumn,!ascSort);
#else
	horizontalHeader()->setSortIndicator(mSortColumn, ascSort);
#endif
    }
    constructorDone = TRUE;
    fitHeadersToWidth();
    refresh();
}

void TodoTable::saveSettings()
{
    QMap<int,QCString> keyToIdentifier = PimTask::keyToIdentifierMap();
    QHeader *header = horizontalHeader();
    QStringList fieldList, sizeList;
    for ( int i = 0; i < header->count(); i++) {
	fieldList.append( keyToIdentifier[ headerKeyFields[i]  ]  );
	sizeList.append( QString::number(header->sectionSize(i)) );
    }

#ifdef QTOPIA_DESKTOP
    QSettings settings;
    settings.insertSearchPath( QSettings::Unix, applicationPath );
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    settings.writeEntry( "/palmtopcenter/todolist/fields", fieldList );
    settings.writeEntry( "/palmtopcenter/todolist/colwidths", sizeList );
    settings.writeEntry( "/palmtopcenter/todolist/sortcolumn", mSortColumn );
    settings.writeEntry( "/palmtopcenter/todolist/ascsort", ascSort );
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
#ifndef QTOPIA_DESKTOP
	if ( *iit == PimTask::CompletedField )
	    header->setLabel( i++, Resource::loadPixmap("task-completed"), "" );
	else
	    header->setLabel(i++, trFields[*iit] );
#else
	header->setLabel(i++, trFields[*iit] );
#endif
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
	case PimTask::CompletedField: return 31;
	case PimTask::Status: return 70;
	case PimTask::Description: return 157;
	case PimTask::Priority: return 52;
	case PimTask::PercentCompleted: return 45;
	case PimTask::StartedDate: return 100;
	case PimTask::CompletedDate: return 100;
	default: return 70;
    }
}

int TodoTable::minimumFieldSize(PimTask::TaskFields f)
{
    switch( f ) {
	case PimTask::CompletedField: return 31;
	default: return 40;
    }
}

void TodoTable::headerClicked(int h)
{
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

