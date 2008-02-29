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

#define QTOPIA_INTERNAL_CONTACT_MRE

#include <qpe/categoryselect.h>
#include <qpe/config.h>
#include <qpe/stringutil.h>
#ifdef QWS
#include <qpe/qcopenvelope_qws.h>
#endif

#include <qasciidict.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qapplication.h>

#include "abtable.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <ctype.h> //toupper() for key hack

/*!
  \class AbTable abtable.h

  \brief The AbTable class is a QTable for showing a list of entries.
*/

AbTable::AbTable( const QValueList<int> *order, QWidget *parent, const char *name )
// #ifdef QT_QTABLE_NOHEADER_CONSTRUCTOR
//     : QTable( 0, 0, parent, name, TRUE ),
// #else
    : QTable( parent, name ),
// #endif
      lastSortCol( -1 ),
      asc( TRUE ),
      aba( ContactIO::ReadWrite ),
      intFields( order ),
      currFindRow( -2 ),
      mCat( 0 ),
      showCat(-2),
      prefField(-1)
{
    mCat.load( categoryFileName() );
    setSelectionMode( NoSelection );
    init();
    setSorting( TRUE );
    connect( this, SIGNAL(clicked(int,int,int,const QPoint &)),
             this, SLOT(itemClicked(int,int)) );
}

AbTable::~AbTable()
{
}

void AbTable::init()
{
    setNumRows( 0 );
    setNumCols( 2 );

    horizontalHeader()->setLabel( 0, tr( "Full Name" ));
    horizontalHeader()->setLabel( 1, tr( "Contact" ));
    setLeftMargin( 0 );
    verticalHeader()->hide();
}

void AbTable::paintCell( QPainter *p, int row, int col, 
	const QRect &cr, bool )
{
#if defined(Q_WS_WIN)
    const QColorGroup &cg = ( !drawActiveSelection && style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus ) ? palette().inactive() : colorGroup() );
#else
    const QColorGroup &cg = colorGroup();
#endif


    //PimContact c = aba.at(row);
    PimContact c(*(aba.sortedContacts().at(row)));
    QString text;
    switch(col) {
	case 0: 
	    text = findContactName(c);
	    break;
	case 1:
	    text = findContactContact(c);
    }
    p->save();

    p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Base ) );

    QPen op = p->pen();
    p->setPen(cg.mid());
    p->drawLine( 0, cr.height() - 1, cr.width() - 1, cr.height() - 1 );
    p->drawLine( cr.width() - 1, 0, cr.width() - 1, cr.height() - 1 );
    p->setPen(op);

    if (row == currentRow()) {
	if (col == 0)
	    p->drawLine( 0, 1, 0, cr.height() - 2);
	if (col == numCols() - 1)
	    p->drawLine( cr.width() - 2, 1, cr.width() - 2, cr.height() - 2);

	p->drawLine( 0, cr.height() - 2, cr.width() - 1, cr.height() - 2 );
	p->drawLine( 0, 0, cr.width() - 2, 0 );
    }

    QFont f = p->font();
    QFontMetrics fm(f);
    p->drawText(2,2 + fm.ascent(), text);

    p->restore();
}

void AbTable::setCurrentCell( int row, int col )
{
    int orow = currentRow();
    QTable::setCurrentCell(row, col);
    for (int i = 0; i < numCols(); i++) {
	updateCell(orow, i);
	updateCell(row, i);
    }
}

void AbTable::columnClicked( int col )
{
    if ( !sorting() )
	return;

    if ( lastSortCol == -1 )
	lastSortCol = col;

    if ( col == lastSortCol ) {
	asc = !asc;
    } else {
	lastSortCol = col;
	asc = TRUE;
    }
    resort();
}

void AbTable::resort()
{
    if ( sorting() ) {
	if ( lastSortCol == -1 )
	    lastSortCol = 0;
	sortColumn( lastSortCol, asc, TRUE );
    }
}

PimContact AbTable::currentEntry()
{
    if (aba.sortedContacts().count() == 0)
	return PimContact();
    if (currentRow() >= aba.sortedContacts().count()) {
	setCurrentCell(aba.sortedContacts().count() - 1, currentColumn());
    } else if (currentRow() < 0) {
	setCurrentCell(0, currentColumn());
    }
    return *(aba.sortedContacts().at(currentRow()));
}

void AbTable::setCurrentEntry(QUuid &u)
{
    int rows, row;
    rows = numRows();

    for ( row = 0; row < rows; row++ ) {
	if ( aba.sortedContacts().at(row)->uid() == u) {
	    setCurrentCell(row, currentColumn());
	    break;
	}
    }
}

void AbTable::updateEntry( const PimContact &newContact )
{
    aba.updateContact(newContact);
    refresh();
}

void AbTable::deleteEntry( const PimContact &c )
{
    aba.removeContact(c);
    refresh();
}

void AbTable::deleteCurrentEntry()
{
    aba.removeContact(currentEntry());
    refresh();
}

void AbTable::refresh()
{
    setNumRows(aba.sortedContacts().count());
}

void AbTable::flush()
{
    aba.saveData();
}

void AbTable::reload()
{
    aba.ensureDataCurrent(TRUE);
    refresh();
}

void AbTable::keyPressEvent( QKeyEvent *e )
{

    // next 3 lines are evil.Q
    /*
    char key = toupper( e->ascii() );

    if ( key >= 'A' && key <= 'Z' )
	moveTo( e->text().lower() );
    */

    if (e->text()[0].isLetterOrNumber())
	moveTo( e->text().lower() );

    switch( e->key() ) {
	case Qt::Key_Space:
	case Qt::Key_Return:
	case Qt::Key_Enter:
	    emit details();
	    break;
	default:
	    QTable::keyPressEvent( e );
    }
}

void AbTable::moveTo( const QString &cl )
{
    int rows = numRows();

    /////===========================
    int l, u, r = 0;
    int comparison=0;

    l = 0;
    u = rows;
    while (l < u)
    {
	r = (l + u) / 2;
	QString first = aba.sortedContacts().at(r)->bestLabel().lower();
	//QString first = aba.at(r).bestLabel().lower();
	comparison = Qtopia::compare(cl, first);
	if (comparison < 0)
	    u = r;
	else if (comparison > 0)
	    l = r + 1;
	else
	    break;
    }

    if ( comparison > 0) {
	if ( l == rows )
	    r = rows - 1;// last element... 
	else
	    r++;
    }

    setCurrentCell( r, currentColumn() );
}


QString AbTable::findContactName( const PimContact &entry )
{
    // We use the fileAs, then company, defaultEmail
    return entry.bestLabel();
}

QString AbTable::findContactContact( const PimContact &entry )
{
    const int idList[] = {
	Qtopia::Emails,
	Qtopia::BusinessMobile,
	Qtopia::BusinessPhone,
	Qtopia::BusinessStreet,
	Qtopia::Company,
	Qtopia::HomeMobile,
	Qtopia::HomePhone,
	Qtopia::HomeStreet,
	-1
    };

    QString value("");

    bool multiValue = qApp->desktop()->width() > 400;
    int infoCount = multiValue ? 3 : 1;
    int count = 0;

    // try to get the preferred entry
    if ( prefField >= 0 ) {
	value = getField( entry, prefField );
	if ( !value.isEmpty() )
	    count++;
    }

    // try to get a sensible entry
    int i = 0;
    while ( idList[i] >= 0 && count < infoCount ) {
	if ( idList[i] != prefField ) {
	    QString v = getField( entry, idList[i] );
	    if ( !v.isEmpty() ) {
		if ( multiValue && count )
		    value += ", " + v;
		else
		    value = v;
		count++;
	    }
	}
	i++;
    }

    // Try to find anything
    if ( value.isEmpty() ) {
	for ( QValueList<int>::ConstIterator it = intFields->begin();
		it != intFields->end(); ++it ) {
	    value = getField( entry, *it );
	    if ( !value.isEmpty() )
		break;
	}
    }

    return value;
}

QString AbTable::getField( const PimContact &entry, int id )
{
    return entry.field(id);
}

void AbTable::addEntry( const PimContact &newCnt )
{
    aba.addContact(newCnt);
    refresh();
}

// conservative on the yes vote.
bool AbTable::hasEntry( const PimContact &c )
{
    QString regExp(c.fileAs());
    QList<PimContact>& contacts = (QList<PimContact> &)aba.contacts();

    QListIterator<PimContact> it(contacts);
    for (; it.current(); ++it) {
	if (it.current()->match(regExp)) {
	    if ( c.toRichText() == (*it)->toRichText() ) {   //expensive compare
		return TRUE;
	    }
	}
    }
    return FALSE;
}

#if QT_VERSION <= 230
#ifndef SINGLE_APP
void QTable::paintEmptyArea( QPainter *p, int cx, int cy, int cw, int ch )
{
    // Region of the rect we should draw
    QRegion reg( QRect( cx, cy, cw, ch ) );
    // Subtract the table from it
    reg = reg.subtract( QRect( QPoint( 0, 0 ), tableSize() ) );
    // And draw the rectangles (transformed as needed)
    QArray<QRect> r = reg.rects();
    for (unsigned int i=0; i<r.count(); i++)
        p->fillRect( r[i], colorGroup().brush( QColorGroup::Base ) );
}
#endif
#endif


int AbTable::rowHeight( int ) const
{
    return 18;
}

int AbTable::rowPos( int row ) const
{
    return 18*row;
}

int AbTable::rowAt( int pos ) const
{
    return QMIN( pos/18, numRows()-1 );
}

static bool contactCompare( const PimContact &cnt, const QRegExp &r, int category );

void AbTable::slotDoFind( const QString &findString, int category )
{
    static bool wrapAround = false;
    if ( currFindString != findString ) {
	currFindRow = -1;
	wrapAround = false;
    }
    currFindString = findString;
    if ( currFindRow < -1 )
	currFindRow = currentRow() - 1;
    clearSelection( TRUE );
    int rows,
        row;
    QRegExp r( findString );
    r.setCaseSensitive( FALSE );
    rows = numRows();

    for ( row = currFindRow + 1; row < rows; row++ ) {
	if ( contactCompare( *(aba.sortedContacts().at(row)), r, category ) )
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
	emit findFound();
	wrapAround = true;
    }
}

static bool contactCompare( const PimContact &cnt, const QRegExp &r, int category )
{
    bool returnMe;
    QArray<int> cats;
    cats = cnt.categories();

    returnMe = false;
    if ( (category == -1 && cats.count() == 0) || category == -2 )
	returnMe = cnt.match( r );
    else {
	int i;
	for ( i = 0; i < int(cats.count()); i++ ) {
	    if ( cats[i] == category ) {
		returnMe = cnt.match( r );
		break;
	    }
	}
    }
    return returnMe;
}

void AbTable::fitColumns()
{
    int contentsWidth = visibleWidth();
    int n = numCols();
    int pw = n == 3 ? columnWidth(2) : 0;
    setColumnWidth( 0, contentsWidth - contentsWidth / 2 );
    setColumnWidth( 1, contentsWidth / 2 - pw );
}

void AbTable::show()
{
    fitColumns();
    QTable::show();
}

void AbTable::setChoiceNames( const QStringList& list)
{
    choicenames = list;
    if ( choicenames.isEmpty() ) {
	// hide pick column
	setNumCols( 2 );
    } else {
	// show pick column
	setNumCols( 3 );
	setColumnWidth( 2, fontMetrics().width(tr( "Pick" ))+8 );
	horizontalHeader()->setLabel( 2, tr( "Pick" ));
    }
    fitColumns();
}

void AbTable::itemClicked(int,int col)
{
    if ( col == 2 ) {
	return;
    } else {
	emit details();
    }
}

QStringList AbTable::choiceNames() const
{
    return choicenames;
}

void AbTable::setChoiceSelection(int /*index*/, const QStringList& /*list*/)
{
    /* ######

	QString selname = choicenames.at(index);
	for (each row) {
	    PimContact *c = contactForRow(row);
	    if ( list.contains(c->email) ) {
		list.remove(c->email);
		setText(row, 2, selname);
	    }
	}
	for (remaining list items) {
	    PimContact *c = new contact(item);
	    setText(newrow, 2, selname);
	}

    */
}

QStringList AbTable::choiceSelection(int /*index*/) const
{
    QStringList r;
    /* ######

	QString selname = choicenames.at(index);
	for (each row) {
	    PimContact *c = contactForRow(row);
	    if ( text(row,2) == selname ) {
		r.append(c->email);
	    }
	}

    */
    return r;
}

void AbTable::setPreferredField( int id )
{
    prefField = id;
}

void AbTable::setShowCategory( int c )
{
    showCat = c;
    aba.setFilter(c);
}

QString AbTable::categoryLabel( int id )
{
    // This is called seldom, so calling a load in here
    // should be fine.
    mCat.load( categoryFileName() );
    if ( id == -1 )
	return tr( "Unfiled" );
    else if ( id == -2 )
	return tr( "All" );
    return mCat.label( "Address Book", id );
}


QStringList AbTable::categories()
{
    mCat.load( categoryFileName() );
    QStringList categoryList = mCat.labels( "Address Book" );
    return categoryList;
}

void AbTable::setPaintingEnabled( bool e )
{
    if ( e != enablePainting ) {
	if ( !enablePainting ) {
	    enablePainting = true;
	    rowHeightChanged( 0 );
	    viewport()->update();
	} else {
	    enablePainting = false;
	}
    }
}

void AbTable::rowHeightChanged( int row )
{
    if ( enablePainting )
	QTable::rowHeightChanged( row );
}
