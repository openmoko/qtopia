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

#define QTOPIA_INTERNAL_CONTACT_MRE

#include <qlabel.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <qvaluelist.h>
#include <qevent.h>
#include <qasciidict.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qapplication.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qlist.h>
#include <qlistview.h>
#include <qaction.h>
#include <qsimplerichtext.h>
#include <qpixmapcache.h>

#ifdef QTOPIA_DESKTOP
#include <qdconfig.h>
#include <qsettings.h>
#include <qapplication.h>
#else
#include <qtopia/qpeapplication.h>
#endif

#include <qtopia/pim/private/contactxmlio_p.h>
#include <qtopia/categoryselect.h>
#include <qtopia/config.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <qtopia/stringutil.h>
#include <qtopia/timestring.h>
#include <qtopia/image.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#include <qtopia/contextbar.h>
#include <qtopia/phone/phone.h>
#include <qtopia/phone/phonebook.h>
#endif

//#include "addressbook.h"
#include "abtable_p.h"

#include <errno.h>
#include <fcntl.h>
#ifndef Q_OS_WIN32
 #include <unistd.h>
#endif
#include <stdlib.h>

#include <ctype.h> //toupper() for key hack

const int AbUtil::ImageHeight = 30;
const int AbUtil::ImageWidth = 24;

const int AbTable::FREQ_CONTACT_FIELD = PimContact::ContactFieldsEnd;

static Qt::ButtonState bState = Qt::NoButton;
static int selectionBeginRow = -1;
static bool constructorDone = FALSE;

static QString applicationPath;
/*!
  \class AbTable abtable.h
  \brief The AbTable class shows list of contacts.
  \internal
*/

AbTable::AbTable( ContactXmlIO *c, QWidget *parent, const char *name, const char *appPath )
    : QTable( 0, 0, parent, name ),
      showCat(-2),
      prefField(PimContact::DefaultEmail)
{
    // TODO : connect signals for 'personal' changes

    applicationPath = appPath;

    fontChange(font());
    paletteChange(palette());

    contacts = c;

    mSel = NoSelection;

    QTable::setSelectionMode( QTable::NoSelection );

    setLeftMargin( 0 );
#ifndef QTOPIA_DESKTOP
    setFrameStyle( NoFrame );
#endif
#ifdef PACK_IN_1_COLUMN
    generic = Resource::loadPixmap("addressbook/generic-contact");
    setTopMargin( 0 );
    setShowGrid(FALSE);
    horizontalHeader()->hide();
    sim = Resource::loadPixmap("addressbook/sim-contact");
#endif
    verticalHeader()->hide();
    setSorting( TRUE );
    connect( this, SIGNAL(clicked(int,int,int,const QPoint&)),
             this, SLOT(itemClicked(int,int)) );
    connect( this, SIGNAL( doubleClicked(int,int,int,const QPoint&) ),
	     this, SLOT( slotDoubleClicked(int,int,int,const QPoint&) ) );
    connect( this, SIGNAL( currentChanged(int,int) ),
             this, SLOT( slotCurrentChanged(int,int) ) );

    mAscending = FALSE;
    mSortColumn = 
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_DESKTOP)
	-1
#else
	0
#endif
	;

#ifdef QTOPIA_DESKTOP
    readSettings();
#else
    QTimer::singleShot(0, this, SLOT(readSettings()) );
#endif

#ifdef QTOPIA_PHONE
    setHScrollBarMode( QScrollView::AlwaysOff );
    mSimContacts.setAutoDelete( TRUE );
#endif
}


AbTable::~AbTable()
{
    saveSettings();
#ifdef QTOPIA_PHONE
    mSimContacts.clear();
#endif
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
    while ( row <= toRow ) 
    {
	mSelected.append( mSortedContacts.at(row)->uid() );
	row++;
    }
}

QString AbTable::fieldText(const PimContact& c, int key)
{
    QString text;
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
    case PimContact::BusinessStreet:
    case PimContact::HomeStreet:
	text = c.field( key ).simplifyWhiteSpace();
	break;
    default:
	text = c.field( key );
    }
    return text;
}

void AbTable::paintCell( QPainter *p, int row, int col,
	const QRect &cr, bool )
{
    if ( !constructorDone )
	return;
    //qDebug("AbTable::paintCell( %d, %d )", row, col );

#if defined(Q_WS_WIN)
    const QColorGroup &cg = ( style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus ) ? palette().inactive() : colorGroup() );
#else
    const QColorGroup &cg = colorGroup();
#endif


    PimContact c(*( mSortedContacts.at(row)));
    int key = headerKeyFields[ col ];
    QString text = fieldText(c,key);

    bool selected = FALSE;
    if ( mSel != NoSelection  && mSelected.find(c.uid()) != mSelected.end() )
	selected = TRUE;
#ifdef QTOPIA_PHONE
    if (row == currentRow())
	selected = TRUE;
#endif
    int kcl = keychars.length();
    int hl = -1;
    if ( kcl ) {
        hl = AbUtil::findPrefixInName(text,keychars);
    }
    

    p->save();
    if ( !selected ) {
	QBrush br;
#ifdef PACK_IN_1_COLUMN
	if ( row % 2 )
	    br = stripebrush;
	else
#endif
	br = cg.brush( QColorGroup::Base );
	p->fillRect( 0, 0, cr.width(), cr.height(), br );
	p->setPen(cg.text());
    } else {
	p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Highlight ) );
	p->setPen(cg.highlightedText() );
    }

#ifdef PACK_IN_1_COLUMN
    QPixmap portrait;

    QString baseDirStr = Global::applicationFileName( "addressbook", "contactimages/" );
    QString pFileName( baseDirStr + c.customField( "photofile" ) );
    if( !c.customField( "photofile" ).isEmpty() )
    {
	QPixmap *cached = QPixmapCache::find( "pimcontact" + c.uid().toString() + "-cfl" );
	if( cached )
	{
	    portrait = *cached;
	}
	else
	{
	    portrait = c.thumbnail( AbUtil::ImageWidth, AbUtil::ImageHeight );
	    if (portrait.isNull()) 
		portrait = generic;
            else 
		QPixmapCache::insert( "pimcontact" + c.uid().toString() + "-cfl", portrait );
	}
    }
    else
    {
	//is it a sim contact or generic?
	if( !c.customField( "SIM_CONTACT" ).isEmpty() )
	    portrait = sim;
	else
	    portrait = generic;
    }

    p->drawPixmap(0,1,portrait);
    int y = 0;
    p->setFont(fnt[0]);
    QFontMetrics fm = p->fontMetrics();
    int sz = 0;
    int w = columnWidth(0);
    QString sep = tr(", ","seperate address/info fields");
    for (int f=0; f < (int)headerKeyFields.count(); f++) {
	bool last = f+1==(int)headerKeyFields.count();
	QString nexttext;
	if ( !last )
	    nexttext = fieldText(c,headerKeyFields[f+1]);
	if ( f==0 && hl==-1 && !nexttext.isEmpty() ) {
	    // hl must be in field field
	    if ( kcl ) {
		int hl2 = AbUtil::findPrefixInName(nexttext,keychars);
		nexttext = nexttext.insert( hl2+kcl, "</u>" );
		nexttext = nexttext.insert( hl2, "<u>" );
	    }
	}
	int align = AlignLeft+WordBreak;
	bool busCard = c.customField("BusinessCard") == "TRUE";
	int bmw = (busCard && f == 0 ? fm.lineSpacing() : 0);
	//scroll bar width already accounted for in columnWidth()
	const int brw = w-AbUtil::ImageWidth-2-bmw,
		  brh = rowH-2-y;
	QRect br = p->boundingRect(AbUtil::ImageWidth+2, y, brw, brh, align, text);

	/* always paint the first(fileas). paint additional information only if it fits */
	if ( f == 0 || 
		(br.width() <= brw && br.height() <= brh) ) 
	{
	    // flush a line/block, go to next smaller size
	    QColorGroup cg = colorGroup();
	    if( selected )
		cg.setColor( QColorGroup::Text, cg.highlightedText() );
	    if ( f==0 && hl != -1) { // draw the underline with QSimpleRichText
		text = text.insert( hl+kcl, "</u>" );
		text = text.insert( hl, "<u>" );
	    }
	    if( f == 0 && busCard )
	    {
		text += QString("<img src=\"%1\" width=%2 height=%3>").arg(
			    Resource::findPixmap("addressbook/business") )
		    .arg( fm.lineSpacing() ).arg( fm.lineSpacing() );
	    }
	    QSimpleRichText srt( text, p->font() );
	    srt.setWidth( brw );
	    srt.draw( p, AbUtil::ImageWidth+2, y, QRegion(br), cg );
	    //p->drawText(ImageWidth+2, y, w-ImageWidth+2, br.height(), align, text);

	    if ( !last ) {
		y += br.height();

		if ( sz++ == 0 ) {
		    p->setFont(fnt[1]);
		} else {
		    p->setFont(fnt[2]);
		}

		QFontMetrics fm = p->fontMetrics();
		if ( fm.lineSpacing() + y >= rowH )
		    break;

		text = nexttext;
	    }
	} else if ( !nexttext.isEmpty() ) {
	    if( !text.isEmpty() )
		text += sep;
	    text += nexttext;
	}
    }
# ifndef QTOPIA_PHONE
    if (row == currentRow())
	p->drawRect(0,0,w,rowH);
#endif
#else
    if ( showGrid() ) {
	p->drawLine( 0, cr.height() - 1, cr.width() - 1, cr.height() - 1 );
	p->drawLine( cr.width() - 1, 0, cr.width() - 1, cr.height() - 1 );
    }

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
#endif

    p->restore();
}

void AbTable::setCurrentCell( int row, int col)
{
    //qDebug("--> AbTable::setCurrentCell( %d, %d )", row, col);
#if !defined(QTOPIA_PHONE) || defined(QTOPIA_DESKTOP)
    int orow = currentRow();
#endif
    QTable::setCurrentCell(row, col);
    //qDebug("<-- AbTable::setCurrentCell( %d, %d )", row, col);
    //FIXME : QTable calls updateCell itself in this fashion, no need to do it manually 
#if !defined(QTOPIA_PHONE) || defined(QTOPIA_DESKTOP)
    for (int i = 0; i < numCols(); i++) {
	updateCell(orow, i);
	updateCell(row, i);
    }
#endif
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

PimContact AbTable::currentEntry() const
{
    if ( mSortedContacts.count() == 0)
	return PimContact();
    int r = currentRow();
    if (r < 0 || r >= (int)mSortedContacts.count())
	r = 0;
    const PimContact *c = mSortedContacts.at(r);
    PimContact newCnt(*c);
    return newCnt;
}

#ifdef QTOPIA_PHONE
QString AbTable::currentSimEntry() const
{
    if( mCurSimEntry.isNull() && hasCurrentEntry() )
    {
	PimContact cur = currentEntry();
	if( cur.customField("SIM_CONTACT").length() )
	    return cur.fileAs();
    }
    return mCurSimEntry;
}

void AbTable::setCurrentSimEntry( const QString &fa )
{
    mCurSimEntry = fa; //store it, because even if it's not in the list yet it should be when the phonebook updates
    if( fa.isNull() )
	return;

    QList<PimContact> processed;

    QString fileAs = fa.lower();
    int rows = numRows();
    for( int i = 0 ; i < rows ; ++i ) // sorted first
    {
	if( mSortedContacts.at( i )->customField( "SIM_CONTACT" ).isEmpty() )
	{
	    processed.append( mSortedContacts.at( i ) );
	    continue;
	}
	if( mSortedContacts.at( i )->fileAs().lower() == fileAs )
	{
	    setCurrentCell( i, currentColumn() );
	    return;
	}
	else
	{
	    processed.append( mSortedContacts.at( i ) );
	}
    }

    // not in sorted, check all
    for( int i = 0 ; i < (int)mAllContacts.count() ; ++i )
    {
	if( mAllContacts.at( i )->customField( "SIM_CONTACT" ).isEmpty() )
	    continue;
	if( !processed.contains(mAllContacts.at(i)) && mAllContacts.at( i )->fileAs().lower() == fileAs )
	{
	    setKeyChars("");
	    setCurrentCell(i, currentColumn());
	}
    }
}
#endif

void AbTable::setCurrentEntry(const QUuid &u)
{
    int rows, row;
    rows = numRows();

#ifdef QTOPIA_PHONE
    QList<PimContact> processed;
#endif
    for ( row = 0; row < rows; row++ ) { // first check sorted contacts
	if ( mSortedContacts.at(row)->uid() == u) {
	    setCurrentCell(row, currentColumn());
#ifdef QTOPIA_PHONE
	    setCurrentSimEntry( QString::null );
#endif
	    return;
	}
#ifdef QTOPIA_PHONE
	else
	    processed.append( mSortedContacts.at( row ) );
#endif
    }
#ifdef QTOPIA_PHONE
    //not in sorted, check if in global
    for( row = 0 ; row < (int)mAllContacts.count() ; ++row )
    {
	if( !processed.contains( mAllContacts.at( row ) ) 
		&& mAllContacts.at( row )->uid() == u )
	{
	    setKeyChars("");
	    setCurrentCell(row, currentColumn());
	}
    }
#endif
}

void AbTable::selectAll()
{
    if ( mSel == NoSelection || mSel == Single )
	return;

    selectionBeginRow = -1;
    mSelected.clear();
    for ( uint u = 0; u < mSortedContacts.count(); u++ ) {
	mSelected.append( mSortedContacts.at(u)->uid() );
    }
    refresh();
}

QValueList<QUuid> AbTable::selectedContacts() const
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

QValueList<PimContact> AbTable::selected() const
{
    QValueList<PimContact> list;
    if ( mSel == Single ) {
    	if ( hasCurrentEntry() )
	    list.append( currentEntry() );
    } else if ( mSel == Extended ) {
	for ( QValueList<QUuid>::ConstIterator it = mSelected.begin(); it != mSelected.end(); ++it) {
	    list.append( pimForUid( *it ) );
	}

	// set current entry as selected when none is selected
	if ( !list.count() && hasCurrentEntry() )
	    list.append( currentEntry() );
    }

    return list;
}

PimContact AbTable::pimForUid(const QUuid &id) const
{
    for ( uint u = 0; u < mSortedContacts.count(); u++) {
	if ( id == mSortedContacts.at(u)->uid() )
	    return *mSortedContacts.at(u);
    }
    return PimContact();
}

bool AbTable::hasCurrentEntry() const
{
    return mSortedContacts.count() != 0;
}


void AbTable::reload()
{
    //qDebug("AbTable::reload");
//    qDebug("reload callled with %d contacts and sortcol %d", mSortedContacts.count(), mSortColumn);
    mSelected.clear();
    mSortedContacts.clear();
#ifdef QTOPIA_PHONE
    mExistingContactSimData.clear();
#endif

#ifdef QTOPIA_PHONE
    contacts->setSorting( PimContact::FileAs, FALSE );
#else
    if ( mSortColumn > -1 ) {
	contacts->setSorting( headerKeyFields[mSortColumn], mAscending);
    }
#endif

    mAllContacts = contacts->sortedContacts();

#ifdef QTOPIA_PHONE
    if( contacts->filter() == -1 || contacts->filter() == -2 ) // only display sim contacts for unfiled and all
    {
	for( uint i = 0 ; i < mSimContacts.count() ; ++i )
	{
	    bool partOfExisting = FALSE;
	    //go through each contact from the sim card
	    bool partialName = FALSE;
	    QString simContactFileAs = mSimContacts.at(i)->fileAs().lower();
	    if( simContactFileAs[simContactFileAs.length()-1] == AbUtil::SIM_PARTIAL_INDICATOR )
	    {
		partialName = TRUE;
		simContactFileAs = simContactFileAs.left( simContactFileAs.length() -1 );
	    }
	    for( uint j = 0 ; j < mAllContacts.count() ; ++j )
	    {
		if( (partialName  && mAllContacts.at(j)->fileAs().lower().startsWith( simContactFileAs )) ||
		    (!partialName &&  simContactFileAs == mAllContacts.at(j)->fileAs().lower()) )
		{
		    //this sim contact matches an existing contact, merge it
		    mExistingContactSimData[ mAllContacts.at(j)->fileAs().lower() ] = 
								    mSimContacts.at(i);
		    partOfExisting = TRUE;
		}
		if( partOfExisting )
		    //matched the sim contact to a real contact, don't need to look anymore
		    break;
	    }
	    if( !partOfExisting )
		//didn't match sim contact to a realcontact, it becomes a contact standalone
		mAllContacts.append( mSimContacts.at( i ) );
	}
    }
    mAllContacts.sort();
#endif

    refresh();
    emit currentChanged();
}

#ifdef QTOPIA_PHONE
QMap<QString,PimContact *> AbTable::existingContactSimData() const
{
    return mExistingContactSimData;
}

void AbTable::setSimContacts( const QList<PimContact> &simContacts )
{
    QString curSimEntry = currentSimEntry();
    PimContact cur = currentEntry(); // get the current entry before calling clear() on mSimContacts.
    bool hasCur = curSimEntry.length() || hasCurrentEntry();

    mExistingContactSimData.clear();
    mSimContacts.clear();
    mSimContacts = simContacts;
    reload();
    if( hasCur )
    {
	if( curSimEntry.length() )
	    setCurrentSimEntry( curSimEntry );
	else
	    setCurrentEntry( cur.uid() );
    }
}

#endif

QValueList<PimContact> AbTable::all() const
{
    QValueList<PimContact> list;
    const uint len = mSortedContacts.count();
    for( uint i = 0 ; i < len ; ++i )
    {
	PimContact *c = (PimContact *)mSortedContacts.at(i);
	list.append( *c );
    }
    return list;
}

void AbTable::refresh()
{
    //qDebug("AbTable::refresh");
//    qDebug("AbTable::refresh %d contacts", mSortedContacts.count() );
    //fill mSortedContacts with records that match
    mSortedContacts.clear();
    if( keychars.isEmpty() )
    {
	mSortedContacts = mAllContacts;
    }
    else
    {
	const uint len = mAllContacts.count();
	QList<const PimContact> matches;
	for( uint i = 0 ; i < len ; ++i )
	{
	    const PimContact *cnt = mAllContacts.at(i);
	    if( AbUtil::findPrefixInName( cnt->fileAs(), keychars ) != -1
#ifdef QTOPIA_PHONE
	     || AbUtil::findPrefixInName( fieldText(*cnt,headerKeyFields[1]), keychars ) != -1
#endif
		)
	    {
		matches.append( cnt );
	    }
	}
	mSortedContacts.resize( matches.count() );
	const PimContact *cnt;
	for( cnt = matches.first() ; cnt ; cnt = matches.next() )
	    mSortedContacts.append( const_cast<PimContact *>(cnt) );
    }
    setNumRows(mSortedContacts.count());
}

void AbTable::setKeyChars(const QString& k)
{
    if ( keychars != k ) {
#ifdef QTOPIA_PHONE
	if( k.isEmpty() ) //back is go back
	    ContextBar::setLabel( this, Key_Back, ContextBar::Back, 
						    ContextBar::ModalAndNonModal );
	else //back is backspace
	    ContextBar::setLabel( this, Key_Back, ContextBar::BackSpace,
						    ContextBar::ModalAndNonModal );
#endif
	keychars = k;
	refresh();
	setCurrentCell( 0, currentColumn() );
    }
}

void AbTable::keyPressEvent( QKeyEvent *e )
{
    switch( e->key() ) {
#ifdef QTOPIA_PHONE
	case Qt::Key_0:
	case Qt::Key_2: case Qt::Key_3: case Qt::Key_4: case Qt::Key_5:
	case Qt::Key_6: case Qt::Key_7: case Qt::Key_8: case Qt::Key_9:
	    {
		setKeyChars(keychars+e->text());
		return;
	    }
	case Qt::Key_BackSpace:
	    if ( keychars.length() ) {
		setKeyChars(keychars.left(keychars.length()-1));
		return;
	    } else {
		QTable::keyPressEvent( e );
	    }
	    break;
	case Qt::Key_No:
	case Qt::Key_Back:
	{
	    if( !keychars.isEmpty() )
	    {
		//back resets filter
		setKeyChars( QString::null );
		e->accept();
	    }
	    else //keychars empty, go back
	    {
		QTable::keyPressEvent( e );
	    }
	    break;
	}
	case Qt::Key_Select:
	    if( !Global::mousePreferred() ) {
		if ( !isModalEditing() ) {
		    QTable::keyPressEvent(e);
		    break;
		}
	    }
	    // Fall through...
#endif
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

void AbTable::resizeEvent( QResizeEvent *e )
{
    QTable::resizeEvent( e );

    // we receive a resize event from qtable in the middle of the constrution, since
    // QTable::columnwidth does qApp->processEvents.  Ignore this event as it causes
    // all sorts of init problems for us
}

void AbTable::showEvent( QShowEvent *e)
{
    fitToSize();
    QTable::showEvent(e);
}

void AbTable::fitToSize()
{
    // work out the avail width.  May need to subtract scrollbar.
    int w = width() - frameWidth()*2;
    if (contentsHeight() >= (height() - horizontalHeader()->height()) )
	w -= ( style().scrollBarExtent().width() );
    
#ifdef PACK_IN_1_COLUMN
    // Showing  close to a whole number of items sucks if it just wastes space
    QFontMetrics lfm(fnt[0]);
    QFontMetrics sfm(fnt[1]);
    rowH = QMAX(AbUtil::ImageHeight+2, lfm.height() + sfm.height() + 2 );
    verticalScrollBar()->setLineStep(rowH);
#endif

    calcFieldSizes(0, w);
}

void AbTable::calcFieldSizes(int oldSize, int size)
{
#ifdef PACK_IN_1_COLUMN
    Q_UNUSED(oldSize);
    setColumnWidth(0,size);
#else
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
	if ( i == col - 1 && size - accumulated> min )
	    newColLen = size - accumulated;
	else
	    accumulated += newColLen;

	setColumnWidth( i, newColLen );
    }

    constructorDone = TRUE;
#endif
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
	QString first = mSortedContacts.at(r)->bestLabel().lower();
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
		value = v.simplifyWhiteSpace();
		break;
	    }
	}
    }

    return value;
}

QString AbTable::getField( const PimContact &entry, int key )
{
    // Not clear why this is preferred over fieldText

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
    QColorGroup::ColorRole empty = QColorGroup::Base;
#ifdef QTOPIA_PHONE
    if ( /*style().guiStyle() == KeyPadStyle && */ !isModalEditing() )
	empty = QColorGroup::Background;
#endif
    for (unsigned int i=0; i<r.count(); i++)
        p->fillRect( r[i], colorGroup().brush( empty ) );
}
#endif
#endif

void AbTable::fontChange( const QFont &oldFont )
{
    QFont f = font();
    QFontMetrics fm(f);

#ifdef PACK_IN_1_COLUMN
    fnt[0] = font(); fnt[0].setBold(TRUE);
    fnt[1] = differentFont(font(),-2); fnt[1].setBold(FALSE);
    fnt[2] = differentFont(fnt[1],-2);

    fitToSize();
#else
    rowH = QMAX(18, fm.height() + 2);
#endif

    QTable::fontChange(oldFont);
}

#ifdef PACK_IN_1_COLUMN
void AbTable::paletteChange( const QPalette &p )
{
    QBrush hl = colorGroup().brush( QColorGroup::Highlight );
    QBrush ba = colorGroup().brush( QColorGroup::Base );
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    if (style().extendedBrush(QStyle::AlternateBase).style() != NoBrush)
	stripebrush = style().extendedBrush(QStyle::AlternateBase);
    else 
#endif
	stripebrush = ba;
    QTable::paletteChange(p);
}
#endif

#ifdef PACK_IN_1_COLUMN
QFont AbTable::differentFont(const QFont& start, int step)
{
    int osize = QFontMetrics(start).lineSpacing();
    QFont f = start;
    for (int t=1; t<6; t++) {
	f.setPointSize(f.pointSize()+step);
	step += step < 0 ? -1 : +1;
	QFontMetrics fm(f);
	if ( fm.lineSpacing() != osize )
	    break;
    }
    return f;
}
#endif

int AbTable::rowHeight( int ) const
{
    return rowH;
}

int AbTable::rowPos( int row ) const
{
    return rowH*row;
}

int AbTable::rowAt( int pos ) const
{
    return QMIN( pos/rowH, numRows()-1 );
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
    //FIXME : don't repaint everything. not sure if this is needed for non phone
#if !defined(QTOPIA_PHONE) || defined(QTOPIA_DESKTOP)
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
#else
    Q_UNUSED(row);
#endif

    emit currentChanged();
}


/* simply disabled for now although this might be useful
void AbTable::setPreferredField( int id )
{
    prefField = id;
}
*/

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
    QStringList sizes;

    if ( f.count() == 0 ) {
	f = defaultFields();
	headerKeyFields.clear();
    }

#ifndef PACK_IN_1_COLUMN
    QHeader *header = horizontalHeader();
    // map old sizes to new pos in header list
    for ( int i = 0; i < (int) f.count(); i++) {
	int pos = headerKeyFields.findIndex( *f.at(i) );
	if ( pos > -1 && pos < header->count() )
	    sizes.append( QString::number( header->sectionSize(pos) ) );
	else
	    sizes.append( QString::number( defaultFieldSize( (PimContact::ContactFields) *f.at(i) ) ));
    }
#endif

    setFields(f, sizes);

    if ( isVisible() )
	fitToSize();
}

void AbTable::setFields(QValueList<int> f, QStringList sizes)
{
#ifdef PACK_IN_1_COLUMN
    headerKeyFields = f;
    setNumCols(1);
    mSortColumn = -1;
    Q_UNUSED(sizes);
#else
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
#endif
}

QValueList<int> AbTable::fields()
{
    return headerKeyFields;
}

QValueList<int> AbTable::defaultFields()
{
    QValueList<int> l;

#ifdef QTOPIA_PHONE
    l.append( PimContact::FileAs );
    l.append( PimContact::Company );
    l.append( PimContact::BusinessCity );
#else
    l.append( PimContact::FileAs );
#endif
    l.append( FREQ_CONTACT_FIELD );

    return l;
}

#ifndef PACK_IN_1_COLUMN
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
#endif

void AbTable::readSettings()
{
    QStringList selectedFields, sizeList;

    {
#ifdef QTOPIA_DESKTOP
	QSettings *settings = gQtopiaDesktopConfig->createQSettings();

	selectedFields = settings->readListEntry("/addressbook/fields" );
	sizeList = settings->readListEntry("/addressbook/colwidths" );

	mSortColumn = settings->readNumEntry("/addressbook/sortcolumn", 0);
	mAscending = settings->readBoolEntry("/addressbook/ascending", FALSE);

	gQtopiaDesktopConfig->deleteQSettings();
#else
	Config config( "addressbook" );
	config.setGroup( "View" );

	selectedFields = config.readListEntry("fields", ',');
	sizeList = config.readListEntry("colwidths",',');
	mSortColumn = config.readNumEntry("sortcolumn", 0);
	mAscending = config.readBoolEntry("ascending", FALSE);
#endif
    }

#ifdef QTOPIA_PHONE
    setFields( defaultFields() );
#else
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
#endif

    if ( mSortColumn > -1 ) {
	reload(); 
	horizontalHeader()->setSortIndicator(mSortColumn,!mAscending);
    }
    constructorDone = TRUE;
    fitToSize();
    refresh();
}

void AbTable::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    //qDebug("AbTable::drawContents( %d, %d, %d, %d )", cx, cy, cw, ch );
    QTable::drawContents( p, cx, cy, cw, ch );
}

void AbTable::saveSettings()
{
    QMap<int,QCString> keyToIdentifier = PimContact::keyToIdentifierMap();
    QHeader *header = horizontalHeader();
    QStringList fieldList, sizeList;
    int i;
    for(i = 0; i < (int)header->count(); i++)
	sizeList.append( QString::number(header->sectionSize(i)) );
    
    for(i = 0; i < (int)headerKeyFields.count(); i++)
    {
    	if(headerKeyFields[i] != FREQ_CONTACT_FIELD )
	    fieldList.append( keyToIdentifier[ headerKeyFields[i] ] );
	else
	    fieldList.append( "Contact" ); // No tr
    }

#ifdef QTOPIA_DESKTOP
    QSettings *settings = gQtopiaDesktopConfig->createQSettings();

    settings->writeEntry( "/addressbook/fields", fieldList );
    settings->writeEntry( "/addressbook/colwidths", sizeList );
    settings->writeEntry( "/addressbook/sortcolumn", mSortColumn );
    settings->writeEntry( "/addressbook/ascending", mAscending );

    gQtopiaDesktopConfig->deleteQSettings();
#else
    Config config( "addressbook" );
    config.setGroup( "View" );
#ifndef QTOPIA_PHONE
    config.writeEntry("fields", fieldList, ',' );
#endif
    config.writeEntry("colwidths", sizeList, ',' );
    config.writeEntry("sortcolumn", mSortColumn );
    config.writeEntry("ascending", mAscending );
#endif

}

//------------------------------------------------------------------------------


ContactSelector::ContactSelector( bool allowNew, ContactXmlIO *contacts, QWidget *parent, const char *name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ), mSelectedType( ContactSelector::NoSelection )
{
    setCaption( tr("Select Contacts") );
    QVBoxLayout *l = new QVBoxLayout( this );

    table = new AbTable( contacts, this, "selectorTable" );
    table ->setSelectionMode( AbTable::Extended );
    connect( table, SIGNAL(clicked()), this, SLOT(contactSelected()) );
    table->reload();

    l->addWidget( table );

    if( allowNew )
    {
#ifndef QTOPIA_PHONE
	//TODO pda
#else
	ContextMenu *menu = new ContextMenu( this );
	QAction *actionNew = new QAction( tr( "New" ), Resource::loadIconSet( "new" )
						    , QString::null, 0, this, 0 );
	connect( actionNew, SIGNAL(activated()), this, SLOT(newSelected()) );
	actionNew->addTo( menu );
#endif
    }
#ifndef QTOPIA_DESKTOP
    QPEApplication::setMenuLike( this, TRUE );
#endif
}

void ContactSelector::newSelected()
{
    setSelectedType( SelectedNew );
}

void ContactSelector::contactSelected()
{
    setSelectedType( SelectedContacts );
}

void ContactSelector::setSelectedType( SelectTypes t )
{
    mSelectedType = t;
    accept();
}

ContactSelector::SelectTypes ContactSelector::selectedType() const
{
    return mSelectedType;
}

void ContactSelector::accept()
{
    QDialog::accept();
}

QValueList<PimContact> ContactSelector::selected() const
{
    QValueList<PimContact> selCnts; 
    if( selectedType() == SelectedContacts )
	selCnts = table->selected();
    return selCnts;
}

QSize ContactSelector::sizeHint() const
{
    return QSize( width(), qApp->desktop()->height() );
}

//------------------------------------------------------------------------------

PhoneTypeSelector::PhoneTypeSelector( const PimContact &cnt, 
        const bool showEmpty, const QString &labelText, QWidget *parent, 
        const char *name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ), mContact( cnt )
{
    setCaption(tr("Phone Type"));
    
    //	Make sure that I take up the full size of the screen
    
    QSize ds = qApp->desktop()->size();
    setGeometry( 0, 0, ds.width(), ds.height() );
    
    QVBoxLayout *l = new QVBoxLayout( this );
    l->setAutoAdd( TRUE );
    mLabel = new QLabel( this );
    mPhoneType = new QListView( this );
    mPhoneType->setHScrollBarMode( QScrollView::AlwaysOff );
    mPhoneType->setFixedWidth(width());
    mPhoneType->addColumn( tr("Type") );
    mPhoneType->addColumn( tr("Current Value") );
    mPhoneType->setColumnWidthMode(1, QListView::Manual);
    mPhoneType->setColumnWidth( 1, width()-mPhoneType->columnWidth( 0 ) ); 
    
    mPhoneType->setAllColumnsShowFocus( TRUE );
#ifdef QTOPIA_PHONE
    QFont f = mPhoneType->font();
    f.setPointSize( 12 );
    mPhoneType->setFont( f );
#endif

    mLabel->setText( labelText );

    QListViewItem *item = 0; 
    
    mShowEmpty = showEmpty;

    item = addType(item, mContact.homePhone(), "addressbook/homephone", PimContact::HomePhone);
    mPhoneType->setSelected( item, TRUE );
    item = addType(item, mContact.homeMobile(), "addressbook/homemobile", PimContact::HomeMobile);
    item = addType(item, mContact.homeFax(), "addressbook/homefax", PimContact::HomeFax);
    item = addType(item, mContact.businessPhone(), "addressbook/businessphone", PimContact::BusinessPhone);
    item = addType(item, mContact.businessMobile(), "addressbook/businessmobile", PimContact::BusinessMobile);
    item = addType(item, mContact.businessFax(), "addressbook/businessfax", PimContact::BusinessFax);
    item = addType(item, mContact.businessPager(), "addressbook/businesspager", PimContact::BusinessPager);
    
    connect( mPhoneType, SIGNAL(returnPressed(QListViewItem *)), this, SLOT(accept()) );

    mPhoneType->setFocus();
	
#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() ) {
	if( mPhoneType->isModalEditing() )
	    mPhoneType->setModalEditing( TRUE );
    }
#endif
}

QListViewItem* PhoneTypeSelector::addType(QListViewItem* prevItem,
    QString number, const char* pixmapName, PimContact::ContactFields phoneType)
{
    if(number.isEmpty())
    {
        if(mShowEmpty)
            number = "(empty)";
        else
            return prevItem;
    }
    
    QListViewItem* item = new QListViewItem( mPhoneType, prevItem, "", number);
    item->setPixmap( 0, Resource::loadPixmap(pixmapName) );
    mPhoneType->insertItem( item );
    mItemToPhoneType[item] = phoneType;
    
    return item;
}

/*QString PhoneTypeSelector::verboseIfEmpty( const QString &number )
{
    if( number.isEmpty() )
	return tr("(empty)");
    return number;
}*/

int PhoneTypeSelector::selected() const
{
    QListViewItem *item = mPhoneType->currentItem();
    if( !item )
	return -1;
    return mItemToPhoneType[item];
}

void PhoneTypeSelector::accept()
{
    QDialog::accept();
    if( selected() != -1 )
	emit selected( (PimContact::ContactFields)selected()  );
}


#ifdef QTOPIA_PHONE
/* matches phone keypad input \a needle to string \a haystack, starting
   at \a offset, for n characters,
   returning how many matched consecutively */
int AbUtil::phoneKeyMatchN(const QString& haystack, int offset, const QString& needle, int n)
{
    // XXX should use PkIMMatcher

    static QString phonekey[10];
    if ( phonekey[0].isNull() ) {
	phonekey[0] = " 0";
	phonekey[1] = "1";
	phonekey[2] = "2abcABC";
	phonekey[3] = "3defDEF";
	phonekey[4] = "4ghiGHI";
	phonekey[5] = "5jklJKL";
	phonekey[6] = "6mnoMNO";
	phonekey[7] = "7pqrsQPRS";
	phonekey[8] = "8tuvTUV";
	phonekey[9] = "9wxyzWXYZ";
    }

    for (int i=0; i<n; i++) 
    {
	//does this key press match its corresponding characters?
	if( !needle[i].isDigit() )
	{
	    if( needle[i].lower() != haystack[offset+i].lower() )
		//not a digit, has to match literally
		return i;
	    //otherwise matches, fall through
	}
	else
	{
	    int pk = needle[i].latin1() - '0';
	    if( pk < 0 || pk > 9 )
	    {
		qDebug("BUG : AbUtil::phoneKeyMatchN - Digit in needle doesn't translate to a valid index.");
		return i;
	    }
	    QChar ch = haystack[i+offset].lower();
	    if ( !phonekey[pk].contains(ch) )
		return i; /*ch.unicode() - phonekey[pk][0].unicode();*/
	}
    }

    return n; // match!
}


#endif

#ifdef QTOPIA_PHONE
#include <qtopia/pim/addressbookaccess.h>
PimContact AbUtil::findContactByNumber( const QString& number, int *matchingField, AddressBookAccess *access )
{
    QString value;
    int bestMatchField = -1;
    PimContact matchingContact;
    int bestMatch = 0;
    int match;

    // Search the address book for a match.
    bool ownAccess = FALSE;
    if( !access )
    {
	access = new AddressBookAccess();
	ownAccess = TRUE;
    }
    AddressBookIterator iter(*access);
    for ( ; iter.current(); ++iter ) {
	for( int f = 0; f < numPhoneFields ; ++f ) {
	    value = iter.current()->field( phoneFields[f] );
	    if ( value.length() > 0 ) {
		value = PhoneUtils::resolveLetters( value );
		match = PhoneUtils::matchNumbers( number, value );
		if ( match > bestMatch ) {
		    bestMatch = match;
		    bestMatchField = phoneFields[f];
		    matchingContact = *iter.current();
		}
	    }
	}
    }
    if( ownAccess )
	delete access;

    *matchingField = bestMatchField;
    return matchingContact;
}
#endif

bool isDelim( const QChar &delim )
{
    static const char *delims = " .-()\":";
    static int numDelims = strlen(delims);

    for( int i = 0 ; i < numDelims ; ++i )
	if( delim == delims[i] )
	    return TRUE;
    return FALSE;
}

int AbUtil::findPrefixInName( const QString &name, const QString &prefix )
{
    int nameLen = name.length();
    int prefixLen = prefix.length();
    if( prefixLen > nameLen  || name.isEmpty() || prefix.isEmpty() )
	return -1;
    int numMatchingChars;
    int matchingIndex = -1;
    int startingIndex; 
    for( int i = 0 ; i < nameLen ; ++i )
    {
	while( i < nameLen && isDelim(name[i]) )
	    ++i;
	if( i >= nameLen )
	    break;

	numMatchingChars = 0;
	startingIndex = i;

#ifdef QTOPIA_PHONE
	if( phoneKeyMatchN( name, startingIndex, prefix, prefixLen ) 
		== prefixLen )
#else
	if( name.mid( startingIndex, prefix.length() ).lower() == prefix.lower() )
#endif
	{
	    matchingIndex = startingIndex;
	    break;
	}
	else
	{
	    //goto next token
	    while( i < nameLen && !isDelim(name[i]) )
		++i;
	}
    }
    return matchingIndex;
}

#ifdef QTOPIA_PHONE
//these keys are the custom fields used for sim integration with PimContact
const QString AbUtil::SIM_HP = "SIM_HP";
const QString AbUtil::SIM_HF = "SIM_HF";
const QString AbUtil::SIM_HM = "SIM_HM";
const QString AbUtil::SIM_BP = "SIM_BP";
const QString AbUtil::SIM_BF = "SIM_BF";
const QString AbUtil::SIM_BM = "SIM_BM";
const QString AbUtil::SIM_BPA = "SIM_BPA";

//indicates the file as of a sim contact is only part of the person's actual name
//due to the size constraints of the simcard
const QChar AbUtil::SIM_PARTIAL_INDICATOR = '+';

//these are the phone types of a contact
const int AbUtil::phoneFields[] =
{
    PimContact::HomePhone,
    PimContact::HomeMobile,
    PimContact::BusinessPhone,
    PimContact::BusinessMobile,
    PimContact::BusinessFax,
    PimContact::HomeFax,
    PimContact::BusinessPager
};
const int AbUtil::numPhoneFields = 7;

#include <qtopia/phone/phone.h>
#include <qtopia/phone/phonebook.h>
/*
   converts all the sim card entries specified by list into contacts.
   it integrates numbers with the same name, eg all entries on the sim
   with the text 'John Smith' will end up in the same contact. it tries
   to work out the kind of number for each entry, otherwise just shoves
   it into the next available phone field.
*/
QList<PimContact> AbUtil::phoneBookToContacts( const QValueList<PhoneBookEntry>& list, bool sort)
{
    QList<PimContact> simCnts;

    //convert the phone book entries in the list to contacts.
    QValueList<PhoneBookEntry>::ConstIterator it;
    for( it = list.begin() ; it != list.end() ; ++it )
    {
	QString name = (*it).text().stripWhiteSpace(), number = (*it).number().stripWhiteSpace();
	if( name.isEmpty() || number.isEmpty() )
	    continue;
            
        int insertBefore = -1;

	//do some parsing to determine the type of phone number
	int phoneType = -1;
	AbUtil::parseSimText( name, phoneType );

	// is this sim entry part of an existing contact?
	PimContact *cnt = 0;
	bool existing = FALSE;
	for( int i = 0 ; i < (int)simCnts.count() ; ++i )
	{
	    QString fa = simCnts.at(i)->fileAs();
            if( fa == name )
	    {
		/*
	       it's part of an existing contact. if we haven't parsed a phone 
	       type, put it in the next available spot
	       */
		if( phoneType < 0 )
		{
		    phoneType = AbUtil::phoneFields[0];
		    for( int j = 0 ; j < AbUtil::numPhoneFields ; ++j )
			if( simCnts.at(i)->field( AbUtil::phoneFields[j] )
				.stripWhiteSpace().isEmpty() )
			{
			    phoneType = phoneFields[j];
			    break;
			}
		}
		simCnts.at(i)->setField( phoneType, number );
		existing = TRUE;
		cnt = simCnts.at(i);
		break;
	    }
            else if( sort && insertBefore == -1 && name < fa )
                insertBefore = i;
	}
	if( !existing )
	{
	    PimContact *newContact = new PimContact();
	    newContact->setFileAs( name );
	    newContact->setFirstName( name );
	    if( phoneType < 0 )
		phoneType = AbUtil::phoneFields[0];
	    newContact->setField( phoneType, number );
	    if(insertBefore == -1)
                simCnts.append( newContact );
            else
                simCnts.insert( insertBefore, newContact );
	    cnt = newContact;
	}
	cnt->setCustomField( "SIM_CONTACT", "1" );
	cnt->setCustomField( AbUtil::simKeyForPhoneType( phoneType ), 
		"1" );
    }

    return simCnts;
}

/*
   parses the name and phone number type from the text part
   of a sim entry.
*/
void AbUtil::parseSimText( QString &name, int &phoneType )
{
    name = name.stripWhiteSpace();
    phoneType = -1;
    //get the last token of the name
    QString lastToken;
    int i = name.length()-1;
    for(  ; i > 0 && !name[i].isSpace() && name[i] != '/' ; --i )
	lastToken.prepend( name[i].lower() );
    bool stripLastToken = ( i > 0 && !name[i].isSpace() ) ? TRUE : FALSE;

    lastToken = lastToken.lower();
    if( i > 0 ) // more than one token
    {
	//is it one of our Qtopia sim identifiers?
	phoneType = AbUtil::simTypeTextToPhoneType( lastToken );
	if( phoneType < 0 )
	{
	    //FIXME : should these sim strings be translated?
	    if( lastToken == "h" || lastToken == "hom" || lastToken == "home" )
		phoneType = PimContact::HomePhone;
	    else if( lastToken == "w" || lastToken == "b" || 
		    lastToken == "work" || lastToken == "bus" )
		phoneType = PimContact::BusinessPhone;
	    else if( lastToken == "m" || lastToken == "mob" || 
		    lastToken == "mobile" )
		phoneType = PimContact::HomeMobile;
	}
	if( stripLastToken )
	    name = name.left( name.length()-(lastToken.length()+1) );
    }
}

/* generates the text that should be stored for a sim entry.
   the generated text is just the contact name with '/' + 
   simTypeText appended(see below).
*/
QString AbUtil::generateSimText( const QString &name, const int &phoneType )
{
    if( name.isEmpty() )
	return name;
    //FIXME : Don't know if we should always use our own identifiers
    // would suck for the use case of using our sim in a different phone
    QString simTypeText = simTypeTextForPhoneType( phoneType );
    int limitLength = SIM_TEXT_LIMIT;
    if( simTypeText.length() )
	limitLength = SIM_TEXT_LIMIT - simTypeText.length() - 1; // -1 for the /
    QString limitedName;
    if( (int)name.length() > limitLength ) // name doesn't fit the limit, indicate such
	limitedName = name.left( limitLength-1 ) + SIM_PARTIAL_INDICATOR;
    else
	limitedName = name.left( limitLength );

    if( simTypeText.isEmpty() )
	return limitedName;
    else
	return limitedName + "/" + simTypeText;
}

/*
   sim type is the text appended to the end of the sim entry
   to mark it's type. eg this function would return "HP" for
   a phoneType of PimContact::HomePhone
*/
QString AbUtil::simTypeTextForPhoneType( const int &phoneType )
{
    // do something tricky an vague
    QString simKey = simKeyForPhoneType( phoneType );
    if( simKey.isEmpty() )
	return simKey;
    //4 is the length of SIM_ at the start of the string
    return simKey.right( simKey.length()-4 );
}

/* return the pimcontact phone type given the text type from the sim */
int AbUtil::simTypeTextToPhoneType( const QString &typeText )
{
    QString tu = typeText.upper();
    for( int i = 0 ; i < AbUtil::numPhoneFields ; ++i )
	if( tu == AbUtil::simTypeTextForPhoneType( AbUtil::phoneFields[i] ).upper() )
	    return AbUtil::phoneFields[i];
    return -1;
}

/*
   returns the 'sim key' given the pimcontact phone type.
   the sim key is the string used to store a flag that indicates
   that a particular number of the contact should be stored 
   on the sim card. eg if the user wanted to store the home 
   phone of a contact on the sim 
   PimContact::customField( AbUtil::SIM_HP ) would contain
   a positive value.
*/
QString AbUtil::simKeyForPhoneType( int phoneType )
{
    switch( phoneType )
    {
	case PimContact::HomePhone:
	    return AbUtil::SIM_HP;
	case PimContact::HomeFax:
	    return AbUtil::SIM_HF;
	case PimContact::HomeMobile:
	    return AbUtil::SIM_HM;
	case PimContact::BusinessPhone:
	    return AbUtil::SIM_BP;
	case PimContact::BusinessFax:
	    return AbUtil::SIM_BF;
	case PimContact::BusinessMobile:
	    return AbUtil::SIM_BM;
	case PimContact::BusinessPager:
	    return AbUtil::SIM_BPA;
    }
    return QString::null;
}

uint AbUtil::numbersForSimCount( const PimContact &cnt )
{
    uint n = 0;
    for( int i = 0 ; i < numPhoneFields ; ++i )
    {
	QString k = simKeyForPhoneType( phoneFields[i] );
	if( cnt.customField( k ).length() && cnt.field( phoneFields[i] ).stripWhiteSpace().length() )
	    ++n;
    }
    return n;
}

//note : not numbers that are currently stored on the sim
//just numbers that have been flagged to be stored on the sim
bool AbUtil::hasNumbersForSim( const PimContact &cnt )
{
    for( int i = 0 ; i < numPhoneFields ; ++i )
    {
	QString k = simKeyForPhoneType( phoneFields[i] );
	//number is to be stored on the sim and isn't empty
	if( cnt.customField( k ).length() && cnt.field( phoneFields[i] ).stripWhiteSpace().length() )
	    return TRUE;
    }
    return FALSE;
}

int AbUtil::SIM_TEXT_LIMIT = 20;
int AbUtil::SIM_NUMBER_LIMIT = 60;
void AbUtil::setSimFieldLimits( const QString &limits )
{
    if( limits.isEmpty() )
	return;
    QStringList l = QStringList::split( ",", limits );
    if( l.count() != 2 )
	return;
    AbUtil::SIM_NUMBER_LIMIT = l[0].toInt();
    AbUtil::SIM_TEXT_LIMIT = l[1].toInt();
    qDebug("AbUtil::setSimFieldLimits( %d, %d )", SIM_NUMBER_LIMIT, SIM_TEXT_LIMIT );
}

/*
   syncs a contact with the numbers on the sim card. syncing means going
   through each number in the contact and-
    deleting any number matching the contacts name and number if the user
    doesn't want it on the sim
    or
    adding it to the sim
    or
    updating the entry on the sim
*/
void AbUtil::syncContactWithSim( PimContact &cnt, QValueList<PhoneBookEntry> &curSimList, PhoneBook *pb )
{
    PhoneLine *line = 0;
    if( !pb )
    {
	line = new PhoneLine();
	pb = line->phoneBook();
    }
    /*
       idea for a cleaner algorithm
       -----------------------------
       get the number of sim fields this contact has
       foreach entry in the local sim list
	if this entry's name matches the contact's file as
	    add it to the freeslot list
       while num entries in the free slot list > number of sim fields in contact
	   remove entry from sim
	   remove entry from local list
	   remove entry from free slot list
       for each sim field in contact
	if have a free slot in the free slot list
	    update the slot on the sim card with sim field from contact
	    remove this free slot from free slot list
	else
	    add sim field to simcard
    */

    QValueList<PimContact::ContactFields> simPhoneFields;
    if( AbUtil::hasNumbersForSim( cnt ) )
	for( int i = 0 ; i < numPhoneFields ; ++i )
	    if( cnt.customField( simKeyForPhoneType( phoneFields[i] ) ).length() && cnt.field( phoneFields[i] ).stripWhiteSpace().length() )
		simPhoneFields.append( (PimContact::ContactFields)phoneFields[i] );
    //qDebug("numSimFields is %d", simPhoneFields.count());
    bool flushPhoneBook = FALSE;
    const QString fileAs = cnt.fileAs();
    const QString lowerFileAs = fileAs.lower();
    QValueList<PhoneBookEntry> freeEntries;
    for( QValueList<PhoneBookEntry>::Iterator it = curSimList.begin() ; 
							it != curSimList.end() ; ++it )
    {
	QString simName = (*it).text();
	int simPhoneType = -1;
	parseSimText( simName, simPhoneType );
	bool partialName = FALSE;
	if( simName[simName.length()-1] == AbUtil::SIM_PARTIAL_INDICATOR )
	{
	    partialName = TRUE;
	    simName = simName.left( simName.length()-1 );
	}
	simName = simName.lower();
	//sim entry matches if it's an exact match or partial name
	if( (!partialName && simName == lowerFileAs) || (partialName && lowerFileAs.startsWith( simName )) )
	    freeEntries.append( *it );
    }

    //qDebug("%d free entries available", freeEntries.count());
    while( freeEntries.count() > simPhoneFields.count() )
    {
	QValueList<PhoneBookEntry>::Iterator pos = freeEntries.fromLast();
	pb->remove( (*pos).index(), "SM", FALSE );
	freeEntries.remove( pos );
	curSimList.remove( *pos );
	flushPhoneBook = TRUE;
    }

    //freeEntries.count <= numSimFields
    for( QValueList<PimContact::ContactFields>::Iterator it = simPhoneFields.begin() ;
						    it != simPhoneFields.end() ; ++it )
    {
	if( freeEntries.count() )
	{
	    QValueList<PhoneBookEntry>::Iterator pos = freeEntries.fromLast();
	    //qDebug("updating pos %d with %s", (*pos).index(), cnt.field((int)*it).latin1());
	    pb->update( (*pos).index(), cnt.field( (int)*it ).left( SIM_NUMBER_LIMIT ), 
		    generateSimText( fileAs, *it ), "SM", FALSE );
	    freeEntries.remove( pos );
	}
	else
	{
	    //qDebug("adding %s", cnt.field((int)*it).latin1());
	    pb->add( cnt.field( (int)*it ).left( SIM_NUMBER_LIMIT ), 
					generateSimText( fileAs, *it ), "SM", FALSE );
	}
	flushPhoneBook = TRUE;
    }

#if 0
    /*
    :   go through each item in the current phone book list.
       delete any entries on the sim with this contact's fileas
       that don't have a number that matches any contact number that
       is to be stored on the sim
       if we find one that has this contact's file as and a matching number,
       update it. because the fileas and number are already the same,
       this can only mean it may have a different phone type.
       we remove it from the local simlist after updating, so that other duplicate 
       numbers with different phone types in the contact don't all try and 
       write to the same sim slot because they have the same numbers.
   */
    for( QValueList<PhoneBookEntry>::Iterator it = curSimList.begin() ; 
							    it != curSimList.end() ; )
    {
	QString name = (*it).text();
	int tmp = -1;
	AbUtil::parseSimText( name, tmp );
	if( name.lower() == lowerFileAs )
	{
	    qDebug("%s == %s", name.latin1(), fileAs.latin1());
	    int phoneType = -1;
	    if( hnfs )
	    {
		for( int i = 0 ; i < AbUtil::numPhoneFields ; ++i )
		    if( !cnt.customField( AbUtil::simKeyForPhoneType( 
			AbUtil::phoneFields[i] ) ).isEmpty() && 
			    PhoneUtils::matchPrefix( (*it).number(), 
						cnt.field( AbUtil::phoneFields[i] ) ) )
		    {
			//this number is for the sim and it matches
			phoneType = AbUtil::phoneFields[i];
			break;
		    }
	    }
	    else
		qDebug("%s doesn't have any contacts for sim", fileAs.latin1());
	    //else this contact has no numbers for sim, so it can't match
	    if( phoneType == -1 )
	    {
		qDebug("removing %s %s", fileAs.latin1(), (*it).number().latin1());
		pb->remove( (*it).index(), "SM", FALSE );
		flushPhoneBook = TRUE;
	    }
	    else
	    {
		//this number on the sim matches a number in the contact.
		//if the types don't match, update
		QString simName = (*it).text();
		int simPhoneType = -1;
		AbUtil::parseSimText( simName, simPhoneType );
		if( simPhoneType == -1 || simPhoneType != phoneType )
		{
		qDebug("updating %s %s", fileAs.latin1(), (*it).number().latin1());
		    pb->update( (*it).index(), cnt.field( phoneType ),
		    AbUtil::generateSimText( fileAs, phoneType ), "SM", FALSE );
		    flushPhoneBook = TRUE;
		}
		else
		    qDebug("simPhoneType == %d", simPhoneType);
		//this contact number is already on the sim, don't add it
		nonNewFields.append( (PimContact::ContactFields)phoneType );
	    }
	    //remove from list. either updated ore moved
	    it = curSimList.remove( it );
	}
	//only inrement if fileas doesn't match
	++it;
    }

    /*
       at this point, all obsolete numbers have been removed and existing numbers 
       have been updated. every phone field that has a sim key and is not a non new
       field needs to be added.
   */
    for( int i = 0 ; i < AbUtil::numPhoneFields ; ++i )
    {
	QString simKey = simKeyForPhoneType( AbUtil::phoneFields[i] );
    if( !nonNewFields.contains( (PimContact::ContactFields)AbUtil::phoneFields[i] ))
	qDebug("nonNewFields doesn't contain %d", AbUtil::phoneFields[i] );
    else
	qDebug("nonNewFields does contain %d", AbUtil::phoneFields[i] );
	if( !cnt.customField(simKey).isEmpty() && 
    !nonNewFields.contains( (PimContact::ContactFields)AbUtil::phoneFields[i] ))
	{
	    qDebug("adding %s-%s as a new sim entry", fileAs.latin1(), cnt.field(AbUtil::phoneFields[i]).latin1());
	    // have a new entry for the sim
	    pb->add( cnt.field( AbUtil::phoneFields[i] ), 
			    AbUtil::generateSimText( fileAs, AbUtil::phoneFields[i] ), 
									"SM", FALSE );
	    flushPhoneBook = TRUE;
	}
    }
#endif

    if( flushPhoneBook )
	pb->flush();
    
    if( line )
	delete line;
}

/*!
  Merges a simcontact with a normal contact.
*/
void AbUtil::mergeSimContact( PimContact &cnt, const PimContact &simCnt )
{
    /*
    TODO
    Not sure what to do here. If the contact in the addressbook has a home phone,
    but the simcard also has a contact that matches with a home phone, which one do you use?
    I don't think you want to make the user pick one or the other, because the cntacts
    could be matching because of a coincidental name match (eg 'John Smith').
    For now i'll use whatever's on the simcard if the contact's value is empty or the values match, otherwise
    the sim card version gets ignored.
    */
    for( int i = 0 ; i < AbUtil::numPhoneFields ; ++i )
    {
	int pt = AbUtil::phoneFields[i];
	if( (cnt.field( pt ).isEmpty() && !simCnt.field( pt ).isEmpty()) ||
	    PhoneUtils::matchPrefix( cnt.field( pt ), simCnt.field( pt ) ) )
	{
	    cnt.setField( pt, simCnt.field( pt ) );
	}
	cnt.setCustomField( AbUtil::simKeyForPhoneType( pt ),
	    simCnt.customField( AbUtil::simKeyForPhoneType( pt ) ) );
    }
}

void AbUtil::unmergeSimContact( PimContact &cnt )
{
    for( int i = 0 ; i < AbUtil::numPhoneFields ; ++i )
	cnt.removeCustomField( AbUtil::simKeyForPhoneType(AbUtil::phoneFields[i]) );
}

#endif

#ifdef QTOPIA_DATA_LINKING
#include <qtopia/qdl.h>
#endif
bool AbUtil::compareContacts( const PimContact &cnt1, const PimContact &cnt2 )
{
    // first check standard contact fields.
    for( int i = PimContact::NameTitle ; i != PimContact::ContactFieldsEnd ; ++i )
    {
	if( cnt1.field( i ) != cnt2.field( i ) )
	    return FALSE;
    }
#ifdef QTOPIA_PHONE

    if( cnt1.customField( "SIM_CONTACT" ) != cnt2.customField( "SIM_CONTACT" ) )
	return FALSE;
    for( int i = 0 ; i < numPhoneFields ; ++i )
    {
	QString k = simKeyForPhoneType( phoneFields[i] );
	if( cnt1.customField( k ) != cnt2.customField( k ) )
	    return FALSE;
    }
    if( cnt1.customField("SIM_CONTACT").length() && cnt2.customField("SIM_CONTACT").length() )
	return TRUE; // both sim contacts and all the data a sim contact can have matches.

    if( cnt1.customField( "tone" ) != cnt2.customField( "tone" ) )
	return FALSE;
#endif
    if( cnt1.categories() != cnt2.categories() )
	return FALSE;
    //standard fields are equal. check custom fields we know of
    if( cnt1.customField( "BUSINESS_CONTACT" ) != cnt2.customField( "BUSINESS_CONTACT" ) )
	return FALSE;
    if( cnt1.customField( "photofile" ) != cnt2.customField( "photofile" ) ) // FIXME : same filename doesn't mean same file contents
	return FALSE;
#ifdef QTOPIA_DATA_LINKING
    if( cnt1.customField( QDL::DATA_KEY ) != cnt2.customField( QDL::DATA_KEY ) )
	return FALSE;
#endif
#ifdef QTOPIA_VOIP
    if( cnt1.customField("VOIP_ID") != cnt2.customField("VOIP_ID") )
        return FALSE;
#endif
    return TRUE;
}

void AbTable::setVBarGeometry( QScrollBar & vbar, int x, int y, int w, int h )
{
	// If a scrollbar is appearing, resize the content so that horizontal
	// scrolling is not needed.
	if(contentsWidth() != visibleWidth())
		resizeContents(visibleWidth(), contentsHeight());
		
	QScrollView::setVBarGeometry(vbar, x, y, w, h);
}
