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

#include <qtopia/categoryselect.h>
#include <qtopia/config.h>
#include <qtopia/stringutil.h>
#include <qtopia/timestring.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif

#ifdef QTOPIA_DESKTOP
#include <qsettings.h>
#endif

#include <qasciidict.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qapplication.h>
#include <qstyle.h>
#include <qtimer.h>

#include "abtable.h"

#include <errno.h>
#include <fcntl.h>
#ifndef Q_OS_WIN32
 #include <unistd.h>
#endif
#include <stdlib.h>


#include <ctype.h> //toupper() for key hack

static int RowHeight = 18;

const int AbTable::FREQ_CONTACT_FIELD = PimContact::ContactFieldsEnd;

static Qt::ButtonState bState = Qt::NoButton;
static int selectionBeginRow = -1;
static bool constructorDone = FALSE;

static QString applicationPath;
/*!
  \class AbTable abtable.h

  \brief The AbTable class is a QTable for showing a list of entries.
*/

AbTable::AbTable(ContactXmlIO *c, QWidget *parent, const char *name, const char *appPath )
    : QTable( 0, 0, parent, name ),
      currFindRow( -2 ),
      showCat(-2),
      prefField(PimContact::DefaultEmail)
{
    applicationPath = appPath;

    QFont f = font();
    QFontMetrics fm(f);
    RowHeight = QMAX(18, fm.height() + 2);

    contacts = c;

    mSel = NoSelection;
    QTable::setSelectionMode( QTable::NoSelection );

    setLeftMargin( 0 );
#ifndef QTOPIA_DESKTOP
    setFrameStyle( NoFrame );
#endif
    verticalHeader()->hide();
    setSorting( TRUE );
    connect( this, SIGNAL(clicked(int,int,int,const QPoint &)),
             this, SLOT(itemClicked(int,int)) );
    connect( this, SIGNAL( doubleClicked( int, int, int, const QPoint & ) ),
	     this, SLOT( slotDoubleClicked( int, int, int, const QPoint & ) ) );
    connect( this, SIGNAL( currentChanged( int, int ) ),
             this, SLOT( slotCurrentChanged( int, int ) ) );

    mAscending = FALSE;
    mSortColumn = -1;
    readSettings();
    constructorDone = TRUE;
}


AbTable::~AbTable()
{
    saveSettings();
}

void AbTable::setSelectionMode(SelectionMode m)
{
    if ( m == NoSelection && mSel != m ) {
	mSelected.clear();
	refresh();
    }

    mSel = m;
}

// We clear the selection and loop through all the rows since currentChanged
// will skip some rows if you move the pointing device fast enough
void AbTable::setSelection(int fromRow, int toRow)
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
	mSelected.append( contacts->sortedContacts().at(row)->uid() );
	row++;
    }
}

void AbTable::paintCell( QPainter *p, int row, int col,
	const QRect &cr, bool )
{
#if defined(Q_WS_WIN)
    const QColorGroup &cg = ( style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus ) ? palette().inactive() : colorGroup() );
#else
    const QColorGroup &cg = colorGroup();
#endif


    PimContact c(*(contacts->sortedContacts().at(row)));

    bool selected = FALSE;
    if ( mSel != NoSelection  && mSelected.find(c.uid()) != mSelected.end() )
	selected = TRUE;

    //PimContact c = contacts->sortedContacts()at(row);
    QString text;
    int key = headerKeyFields[ col ];
    switch(key) {
    case PimContact::FileAs:
	text = findContactName(c);
	break;
    case FREQ_CONTACT_FIELD:
	text = findContactContact(c);
	break;
    case PimContact::Birthday:
	{
	    QDate d = c.birthday();
	    if ( !d.isNull() )
		text = TimeString::localYMD( d );
	}
	break;
    case PimContact::Anniversary:
	{
	    QDate d = c.anniversary();
	    if ( !d.isNull() )
		text = TimeString::localYMD( d );
	}
	break;
    case PimContact::Gender:
	switch( c.gender() ) {
	    case PimContact::Male: text = tr("Male"); break;
	    case PimContact::Female: text = tr("Female"); break;
	    default: text = ""; break;
	}
	break;
    case PimContact::Notes:
	text = c.field( key ).simplifyWhiteSpace();
	break;
    default:
	text = c.field( key );
    }
    p->save();

    if ( !selected ) {
	p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Base ) );
	p->setPen(cg.text());
    } else {
	p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Highlight ) );
	p->setPen(cg.highlightedText() );
    }

    p->drawLine( 0, cr.height() - 1, cr.width() - 1, cr.height() - 1 );
    p->drawLine( cr.width() - 1, 0, cr.width() - 1, cr.height() - 1 );

    if (row == currentRow()) {
	QPen op = p->pen();
	p->setPen( QColor(black) );

	if (col == 0)
	    p->drawLine( 0, 1, 0, cr.height() - 2);
	if (col == numCols() - 1)
	    p->drawLine( cr.width() - 2, 1, cr.width() - 2, cr.height() - 2);

	p->drawLine( 0, cr.height() - 2, cr.width() - 1, cr.height() - 2 );
	p->drawLine( 0, 0, cr.width() - 2, 0 );

	p->setPen(op);
    }

    QFont f = p->font();
    QFontMetrics fm(f);
    p->drawText(2,2 + fm.ascent(), text);

    p->restore();
}

void AbTable::setCurrentCell( int row, int col)
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
    if ( col != mSortColumn ) {
	mSortColumn = col;
	mAscending = FALSE;
    } else {
	mAscending = !mAscending;
    }
    horizontalHeader()->setSortIndicator(mSortColumn,!mAscending);
    reload();
}

PimContact AbTable::currentEntry()
{
    if (contacts->sortedContacts().count() == 0)
	return PimContact();
    if (currentRow() >= (int)contacts->sortedContacts().count()) {
	setCurrentCell( contacts->sortedContacts().count() - 1, currentColumn());
    } else if (currentRow() < 0) {
	setCurrentCell(0, currentColumn());
    }
    return *(contacts->sortedContacts().at(currentRow()));
}

void AbTable::setCurrentEntry(const QUuid &u)
{
    int rows, row;
    rows = numRows();

    for ( row = 0; row < rows; row++ ) {
	if ( contacts->sortedContacts().at(row)->uid() == u) {
	    setCurrentCell(row, currentColumn());
	    break;
	}
    }
}

void AbTable::selectAll()
{
    if ( mSel == NoSelection || mSel == Single )
	return;

    selectionBeginRow = -1;
    mSelected.clear();
    for ( uint u = 0; u < contacts->sortedContacts().count(); u++ ) {
	mSelected.append( contacts->sortedContacts().at(u)->uid() );
    }
    refresh();
}

QValueList<QUuid> AbTable::selectedContacts()
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

QValueList<PimContact> AbTable::selected()
{
    QValueList<PimContact> list;
    if ( mSel == Single ) {
    	if ( hasCurrentEntry() )
	    list.append( currentEntry() );
    } else if ( mSel == Extended ) {
	for ( QValueList<QUuid>::Iterator it = mSelected.begin(); it != mSelected.end(); ++it) {
	    list.append( pimForUid( *it ) );
	}

	// set current entry as selected when none is selected
	if ( !list.count() && hasCurrentEntry() )
	    list.append( currentEntry() );
    }

    return list;
}

PimContact AbTable::pimForUid(const QUuid &id)
{
    for ( uint u = 0; u < contacts->sortedContacts().count(); u++) {
	if ( id == contacts->sortedContacts().at(u)->uid() )
	    return *contacts->sortedContacts().at(u);
    }
    return PimContact();
}

bool AbTable::hasCurrentEntry()
{
    return contacts->sortedContacts().count() != 0;
}


void AbTable::reload()
{
//    qDebug("reload callled with %d contacts and sortcol %d", c.count(), mSortColumn);
    mSelected.clear();

    if ( mSortColumn > -1 ) {
	contacts->setSorting( headerKeyFields[mSortColumn], mAscending);
//	contacts->sortedContacts()sort();
    }

    refresh();
    emit currentChanged();
}

void AbTable::refresh()
{
//    qDebug("AbTable::refresh %d contacts", contacts->sortedContacts()count());
    setNumRows(contacts->sortedContacts().count());
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
	    emit clicked();
	    break;
	default:
	    QTable::keyPressEvent( e );
    }
}

void AbTable::contentsMousePressEvent( QMouseEvent *e )
{
    if ( mSel == Extended ) {
	mSelected.clear();
	bState = e->button();
    }

    QTable::contentsMousePressEvent( e );
}

void AbTable::contentsMouseReleaseEvent( QMouseEvent *e )
{
    if ( mSel == Extended ) {
	selectionBeginRow = -1;
	bState = Qt::NoButton;
    }

    QTable::contentsMouseReleaseEvent( e );
}

static bool needColumnResize = TRUE;

void AbTable::resizeEvent( QResizeEvent *e )
{
    QTable::resizeEvent( e );

    // we receive a resize event from qtable in the middle of the constrution, since
    // QTable::columnwidth does qApp->processEvents.  Ignore this event as it causes
    // all sorts of init problems for us
    if ( !constructorDone )
	return;
}

void AbTable::showEvent( QShowEvent *e)
{
    QTable::showEvent(e);
    if ( needColumnResize ) {
	needColumnResize = FALSE;
	fitHeadersToWidth();
    }
}

void AbTable::fitHeadersToWidth()
{
    // work out the avail width.  May need to subtract scrollbar.
    int w = width() - frameWidth();
    if (contentsHeight() >= (height() - horizontalHeader()->height()) )
	w -= ( style().scrollBarExtent().width() );
    
    calcFieldSizes(0, w);
}

void AbTable::calcFieldSizes(int oldSize, int size)
{
    constructorDone = FALSE; //don't let QTable mess up our logic
    int col = headerKeyFields.count();

    int max = 0;
    int i;
    for ( i = 0; i < col; i++) {
	max += columnWidth(i);
    }
    if ( oldSize < max )
	oldSize = max;

    int accumulated = 0;
    for ( i = 0; i < col; i++) {
	float l = (float) columnWidth( i ) / (float) oldSize;
	float l2 = l * size;
	int newColLen = (int) l2;

	int min = minimumFieldSize( (PimContact::ContactFields) headerKeyFields[i] );
	if ( newColLen < min )
	    newColLen =  min;

	// make sure we fill out the space if there's some integer rounding leftover
	if ( i == col - 1 && size - accumulated - 2> min )
	   newColLen = size - accumulated - 2;
	else
	    accumulated += newColLen;

	setColumnWidth( i, newColLen );
    }

    constructorDone = TRUE;
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
	QString first = contacts->sortedContacts().at(r)->bestLabel().lower();
	//QString first = contacts->sortedContacts().at(r).bestLabel().lower();
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
	PimContact::DefaultEmail,
	PimContact::BusinessMobile,
	PimContact::BusinessPhone,
	PimContact::BusinessStreet,
	PimContact::Company,
	PimContact::HomeMobile,
	PimContact::HomePhone,
	PimContact::HomeStreet,
	-1
    };

    QString value;

    // try to get the preferred entry
    switch (prefField) {
	default:
	case PimContact::DefaultEmail:
	    value = getField(entry, PimContact::DefaultEmail);
	    break;
	case PimContact::HomePhone:
	    value = getField(entry, PimContact::HomePhone);
	    if (value.isEmpty())
		value = getField(entry, PimContact::HomeMobile);
	    if (value.isEmpty())
		value = getField(entry, PimContact::BusinessMobile);
	    break;
	case PimContact::HomeMobile:
	    value = getField(entry, PimContact::HomeMobile);
	    if (value.isEmpty())
		value = getField(entry, PimContact::BusinessMobile);
	    if (value.isEmpty())
		value = getField(entry, PimContact::HomePhone);
	    break;
	case PimContact::BusinessPhone:
	    value = getField(entry, PimContact::BusinessPhone);
	    if (value.isEmpty())
		value = getField(entry, PimContact::BusinessMobile);
	    if (value.isEmpty())
		value = getField(entry, PimContact::HomeMobile);
	    break;
	case PimContact::BusinessMobile:
	    value = getField(entry, PimContact::BusinessMobile);
	    if (value.isEmpty())
		value = getField(entry, PimContact::HomeMobile);
	    if (value.isEmpty())
		value = getField(entry, PimContact::BusinessPhone);
	    break;
    }
    if (value.isEmpty()) {
	// try to get a sensible entry
	for (int i = 0; idList[i] >= 0; i++) {
	    QString v = getField( entry, idList[i] );
	    if ( !v.isEmpty() ) {
		value = v;
		break;
	    }
	}
    }

    return value;
}

QString AbTable::getField( const PimContact &entry, int key )
{
    QString res = entry.field( key );
    QString type;
 
    if ( !res.isEmpty() )
	switch ( key ) {
	case PimContact::DefaultEmail:
	    // type = tr("E: ","Short title for Default email address");
	    // break;
	    return QString(res); // Had to do this because of text freeze - Luke
	case PimContact::HomePhone:
	    type = tr("H: ","Short title for Home phone");
	    break;
	case PimContact::BusinessPhone:
	    type = tr("B: ","Short title for Business phone");
	    break;
	case PimContact::HomeMobile:
	    type = tr("HM: ","Short title for Home mobile");
	    break;
	case PimContact::BusinessMobile:
	    type = tr("BM: ","Short title for Business mobile");
	    break;
	}
    if ( !type.isEmpty() )
	res = type + res;
    return res;
}

static bool contactCompare( const PimContact &cnt, const QRegExp &r, int category );

void AbTable::findNext( const QString &findString, int category )
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
	if ( contactCompare( *(contacts->sortedContacts().at(row)), r, category ) )
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
	setCurrentCell( currFindRow, currentColumn());
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

void AbTable::fontChange( const QFont &oldFont )
{
    QFont f = font();
    QFontMetrics fm(f);
    RowHeight = QMAX(18, fm.height() + 2);

    QTable::fontChange(oldFont);
}

int AbTable::rowHeight( int ) const
{
    return RowHeight;
}

int AbTable::rowPos( int row ) const
{
    return RowHeight*row;
}

int AbTable::rowAt( int pos ) const
{
    return QMIN( pos/RowHeight, numRows()-1 );
}

void AbTable::show()
{
    QTable::show();
}

void AbTable::itemClicked(int,int /* col */)
{
    emit clicked();
}

void AbTable::slotDoubleClicked(int, int, int, const QPoint &)
{
    emit doubleClicked();
}

void AbTable::slotCurrentChanged( int row, int )
{
    bool needRefresh = FALSE;

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
	refresh();
    }

    emit currentChanged();
}


void AbTable::setPreferredField( int id )
{
    prefField = id;
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

void AbTable::setFields(QValueList<int> f)
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
	if ( pos > -1 && pos < header->count() )
	    sizes.append( QString::number( header->sectionSize(pos) ) );
	else
	    sizes.append( QString::number( defaultFieldSize( (PimContact::ContactFields) *f.at(i) ) ));
    }

    setFields(f, sizes);

    needColumnResize = TRUE;
    if ( isVisible() )
	fitHeadersToWidth();
}

void AbTable::setFields(QValueList<int> f, QStringList sizes)
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
    QValueList<int>::Iterator iit;
    int i = 0;
    for (iit = f.begin(); iit != f.end(); ++iit) {
	if ( *iit == prevSortKey ) {
	    mSortColumn = i;
	    break;
	}
	i++;
    }
    setNumCols( f.count() );

    QMap<int, QString> trFields = PimContact::trFieldsMap();
    i = 0;
    for (iit = f.begin(); iit != f.end(); ++iit) {
	if ( *iit == FREQ_CONTACT_FIELD )
	    header->setLabel(i, tr("Contact") );
	else
	    header->setLabel(i, trFields[*iit] );
	i++;
    }

    i = 0;
    for (QStringList::ConstIterator it = sizes.begin(); it != sizes.end(); ++it) {
	if ( i < (int) f.count() )
	    setColumnWidth(i, (*it).toInt() );
	i++;
    }

    if ( mSortColumn > -1 )
	header->setSortIndicator(mSortColumn, !mAscending);
}

QValueList<int> AbTable::fields()
{
    return headerKeyFields;
}

QValueList<int> AbTable::defaultFields()
{
    QValueList<int> l;

    l.append( PimContact::FileAs );
    l.append( FREQ_CONTACT_FIELD );

    return l;
}

int AbTable::defaultFieldSize(PimContact::ContactFields f)
{
    switch( f ) {
	case PimContact::FileAs: return 119;
	case FREQ_CONTACT_FIELD: return 119;
	default: return 80;
    }
}

int AbTable::minimumFieldSize(PimContact::ContactFields)
{
    return 40;
}

void AbTable::readSettings()
{
    QStringList selectedFields, sizeList;

    {
#ifdef QTOPIA_DESKTOP
	QSettings settings;
	settings.insertSearchPath( QSettings::Unix, applicationPath );
	settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

	selectedFields = settings.readListEntry("/palmtopcenter/addressbook/fields" );
	sizeList = settings.readListEntry("/palmtopcenter/addressbook/colwidths" );

	mSortColumn = settings.readNumEntry("/palmtopcenter/addressbook/sortcolumn", 0);
	mAscending = settings.readBoolEntry("/palmtopcenter/addressbook/ascending", FALSE);
#else
	Config config( "addressbook" );
	config.setGroup( "View" );

	selectedFields = config.readListEntry("fields", ',');
	sizeList = config.readListEntry("colwidths",',');
	mSortColumn = config.readNumEntry("sortcolumn", 0);
	mAscending = config.readBoolEntry("ascending", FALSE);
#endif
    }

    if ( !selectedFields.count() ) {
	setFields( defaultFields() );
    } else {
	QMap<QCString, int> identifierToKey = PimContact::identifierToKeyMap();
	for ( QStringList::Iterator it = selectedFields.begin(); it != selectedFields.end(); ++it) {
	    if ( *it == "Contact" ) { // No tr
		headerKeyFields.append( FREQ_CONTACT_FIELD );
	    } else {
		int field = identifierToKey[ (*it).data() ];
		headerKeyFields.append( field );
	    }
	}

	setFields( headerKeyFields, sizeList );
    }

    if ( mSortColumn > -1 ) {
	reload(); 
	horizontalHeader()->setSortIndicator(mSortColumn,!mAscending);
    }
}

void AbTable::saveSettings()
{
    QMap<int,QCString> keyToIdentifier = PimContact::keyToIdentifierMap();
    QHeader *header = horizontalHeader();
    QStringList fieldList, sizeList;
    for ( int i = 0; i < header->count(); i++) {
	if ( headerKeyFields[i] != FREQ_CONTACT_FIELD )
	    fieldList.append( keyToIdentifier[ headerKeyFields[i]  ]  );
	else
	    fieldList.append( "Contact" ); // No tr

	sizeList.append( QString::number(header->sectionSize(i)) );
    }

#ifdef QTOPIA_DESKTOP
    QSettings settings;
    settings.insertSearchPath( QSettings::Unix, applicationPath );
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    settings.writeEntry( "/palmtopcenter/addressbook/fields", fieldList );
    settings.writeEntry( "/palmtopcenter/addressbook/colwidths", sizeList );
    settings.writeEntry( "/palmtopcenter/addressbook/sortcolumn", mSortColumn );
    settings.writeEntry( "/palmtopcenter/addressbook/ascending", mAscending );
#else
    Config config( "addressbook" );
    config.setGroup( "View" );
    config.writeEntry("fields", fieldList, ',' );
    config.writeEntry("colwidths", sizeList, ',' );
    config.writeEntry("sortcolumn", mSortColumn );
    config.writeEntry("ascending", mAscending );
#endif

}

