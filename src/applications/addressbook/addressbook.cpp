/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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

#define QTOPIA_INTERNAL_FD
#define QTOPIA_INTERNAL_FILEOPERATIONS

#include "abeditor.h"
#include "ablabel.h"
#include <qtopia/pim/private/abtable_p.h>
#include <qtopia/pim/private/contactfieldselector_p.h>
#include "addressbook.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/qdawg.h>
#ifdef QTOPIA_PHONE
#include <qtopia/speeddial.h>
#endif
#include <qtopia/pim/contact.h>
#include <qtopia/global.h>
#include <qtopia/resource.h>
#include <qtopia/ir.h>
#include <qtopia/qpemessagebox.h>
#include <qtopia/categoryselect.h>
#include <qtopia/services.h>
#include <qtopia/categories.h>
#include <qtopia/applnk.h>
#include <qtopia/image.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/qpemenubar.h>
#include <qtopia/qpetoolbar.h>
#ifdef QTOPIA_PHONE
# include <qtopia/categorydialog.h>
# include <qtopia/contextbar.h>
#endif
#ifdef QTOPIA_DATA_LINKING
#include <qtopia/qdl.h>
#endif
#ifdef QTOPIA_PHONE
#include <qtopia/phone/phone.h>
#include <qtopia/phone/phonebook.h>
#endif

#include <qdialog.h>
#include <qdir.h>
#include <qfile.h>
#include <qimage.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qwhatsthis.h>
#include <qcstring.h>
#include <qmessagebox.h>
#include <qpixmapcache.h>

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#ifndef Q_OS_WIN32
#   include <unistd.h>
#endif

#include <qdatetime.h>

static bool constructorDone = FALSE;

static const struct {
    const char* desc;
    const char* fmt;
} fileas_opt[] = {
    { QT_TRANSLATE_NOOP("AbSettings","First Last or Company"),
	"FirstName LastName | Company" }, // No tr
    { QT_TRANSLATE_NOOP("AbSettings","Last, First or Company"),
	"LastName , _ FirstName | Company" }, // No tr
    { 0, 0 }
};

class NameLearner {
    // Utility class for efficiently putting names from contacts into word lists.
public:
    NameLearner()
    {
    }

    NameLearner(const PimContact &cnt)
    {
	learn(cnt);
    }

    ~NameLearner()
    {
	const QDawg& dawg = Global::dawg("addressbook-names");
	const QDawg& gldawg = Global::dawg("local");

	QStringList aw;
	QStringList glaw;

	for (QStringList::ConstIterator it=nw.begin(); it!=nw.end(); ++it) {
	    if ( isLetters(*it) ) {
		if ( !dawg.contains(*it) )
		    aw.append(*it);
		if ( !gldawg.contains(*it) )
		    glaw.append(*it);
	    } else {
		// Maybe just spaces (eg. multi-word names)
		QStringList ws = QStringList::split(QChar(' '),*it);
		for (QStringList::ConstIterator lit=ws.begin(); lit!=ws.end(); ++lit) {
		    if ( isLetters(*lit) && !dawg.contains(*lit) )
			aw.append(*lit);
		    if ( isLetters(*lit) && !gldawg.contains(*lit) )
			glaw.append(*lit);
		}
	    }
	}

	Global::addWords("addressbook-names",aw);
	Global::addWords(glaw);
    }

    void learn(const PimContact &cnt)
    {
	static PimContact::ContactFields names[] = {
	    PimContact::FirstName,
	    PimContact::MiddleName,
	    PimContact::LastName,
	    PimContact::Company,
	    PimContact::Spouse,
	    PimContact::Assistant,
	    PimContact::Manager
	};

	for (uint i=0; i<sizeof(names)/sizeof(names[0]); i++) {
	    QString n = cnt.field(names[i]);
	    if ( !n.isEmpty() && !map.find(n) ) {
		nw.append(n);
		map.insert(n,(void*)1);
	    }
	}
    }

private:
    static bool isLetters(const QString& w)
    {
	for (int i=0; i<(int)w.length(); ++i) {
	    if ( !w[i].isLetter() || w[i].unicode() >= 4096/*QDawg limit*/ )
		return FALSE;
	}
	return TRUE;
    }

    QStringList nw;
    QDict<void> map;
};


AbSettings::AbSettings(QWidget *parent, const char *name, bool modal, WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setName("settings");
    QGridLayout *grid = new QGridLayout(this);

#ifdef QTOPIA_PHONE
    Config cfg("Contacts");
    cfg.setGroup("formatting");
    QString curfmt = cfg.readEntry("NameFormat");

    setCaption(tr("Settings"));
    bg = new QButtonGroup(1,Horizontal,tr("File As"),this);
    bg->layout()->setMargin(4);

    for (int i=0; fileas_opt[i].desc; ++i) {
	QRadioButton *rb = new QRadioButton(tr(fileas_opt[i].desc),bg);
	if ( fileas_opt[i].fmt == curfmt || i==0 && curfmt.isEmpty() )
	    rb->setChecked(TRUE);
    }

    grid->addWidget(bg, 0, 0);
#else
    setCaption(tr("Configure columns to display"));

    map = new FieldMap(this);
    grid->addWidget(map, 0, 0);
#endif
}

AbSettings::~AbSettings()
{
}

#ifdef QTOPIA_PHONE
void AbSettings::saveFormat()
{
    Config cfg("Contacts");
    cfg.setGroup("formatting");
    cfg.writeEntry("NameFormat", fileas_opt[bg->id(bg->selected())].fmt);
}

#else

void AbSettings::setCurrentFields(const QValueList<int> &f)
{
    QMap<int,QString> m = PimContact::trFieldsMap();
    m.remove( PimRecord::Categories );
    m.remove( PimContact::LastNamePronunciation );
    m.remove( PimContact::FirstNamePronunciation );
    m.remove( PimContact::CompanyPronunciation );

    m.insert( AbTable::FREQ_CONTACT_FIELD, tr("Contact") );
    map->setFields( m, f);
}
#endif

class LoadIndicator : public QLabel
{
public:   
    LoadIndicator( QWidget *parent = 0, const char *name = 0, WFlags fl = WStyle_Customize | WPaintUnclipped | WStyle_Tool | WType_TopLevel | WStyle_StaysOnTop );
    void center();
    void bottom();

protected:
    void showEvent( QShowEvent *e );
    void paintEvent( QPaintEvent *e );
};


AddressbookWindow::AddressbookWindow( QWidget *parent, const char *name,
				      WFlags f )
    : QMainWindow( parent, name, f ),
      allowCloseEvent( FALSE ),
      mResetKeyChars( FALSE ),
      mCloseAfterView( FALSE ),	
      contacts( ContactIO::ReadWrite ),
      abEditor(0),
      mView(0),
      searchBar(0),
      actionFind(0),
      actionPersonal(0),
      actionSetPersonal(0),
#ifdef QTOPIA_PHONE
      mFindLE(0),
#endif
      syncing(FALSE),
      showingPersonal(FALSE)
{
#ifdef QTOPIA_PHONE
    mSimIndicator = 0;
    mGotSimEntries = FALSE;
    mToggleInternal = FALSE;
#endif
    
#ifdef Q_WS_QWS
    beamfile = Global::tempDir() + "obex";

    QDir d;
    d.mkdir(beamfile);
    beamfile += "/contact.vcf";
#endif

    setCaption( tr("Contacts") );
    setIcon( Resource::loadPixmap( "AddressBook" ) );

    setToolBarsMovable( FALSE );
    setBackgroundMode( PaletteButton );

    // Create Views
    listView = new QVBox( this );

#ifdef QTOPIA_PHONE
    if( Global::mousePreferred() )
	mFindLE = new QLineEdit( listView );
#endif
    abList = new AbTable( &contacts, listView );

#ifdef QTOPIA_PHONE
    if( Global::mousePreferred() ) {
	connect( mFindLE, SIGNAL(textChanged(const QString&)), this, SLOT(search(const QString&)) );
	abList->setFocusPolicy( NoFocus );
	listView->setFocusPolicy( NoFocus );
	mFindLE->installEventFilter( this );
	QPEApplication::setInputMethodHint(mFindLE,"addressbook-names");
    }
#endif

    actionNew = new QAction( tr( "New" ), Resource::loadIconSet( "new" ), QString::null,
			      0, this, 0 );
    connect( actionNew, SIGNAL( activated() ), this, SLOT( slotListNew() ) );
    actionNew->setWhatsThis( tr("Enter a new contact.") );

    actionEdit = new QAction( tr( "Edit" ), Resource::loadIconSet( "edit" ), QString::null,
				  0, this, 0 );
    actionEdit->setWhatsThis( tr("Edit the highlighted contact.") );
    connect( actionEdit, SIGNAL( activated() ), this, SLOT( slotViewEdit() ) );
    actionEdit->setEnabled(FALSE);

    actionTrash = new QAction( tr( "Delete" ), Resource::loadIconSet( "trash" ), QString::null,
		     0, this, 0 );
    actionTrash->setWhatsThis( tr("Delete the selected contacts.") );
    connect( actionTrash, SIGNAL( activated() ), this, SLOT( slotListDelete() ) );
    actionTrash->setEnabled(FALSE);

    actionFind = new QAction( tr( "Find" ), Resource::loadIconSet( "find" ),
                     QString::null, 0, this, 0 );
    actionFind->setWhatsThis( tr("Find a contact.") );
    actionFind->setToggleAction( TRUE );
    connect( actionFind, SIGNAL(toggled(bool)), this, SLOT(slotFind(bool)) );

    QAction *actionShowBCard;
    actionShowBCard = new QAction( tr("Show Business Card"), QString::null, 0, 0, 0, FALSE );
    actionPersonal = actionShowBCard;
    actionShowBCard->setWhatsThis( tr("Show your business card entry.") );
    connect( actionShowBCard, SIGNAL( activated() ), this, SLOT( slotPersonalView() ) );

    QAction *actionSetBCard = new QAction( tr("Set as Business Card"), QString::null, 0, 0, 0, FALSE );
    actionSetPersonal = actionSetBCard;
    actionSetBCard->setWhatsThis( tr("Set current entry as your business card.") );
    connect( actionSetBCard, SIGNAL( activated() ), this, SLOT( markCurrentAsPersonal() ) );

#ifndef QTOPIA_DESKTOP
    mSendService = new SendContactService( this );
    if( mSendService->serverCount() != 0 )
    {
	actionSend = new QAction( tr("Send..."), 
			    Resource::loadIconSet("beam"), 
						QString::null, 0, this, 0 );
	actionSend->setWhatsThis( tr("Send the contact to another person") );
	connect( actionSend, SIGNAL(activated()), this, SLOT(sendContact()) );

	actionSendCat = new QAction( tr("Send All..."), 
			    Resource::loadIconSet("beam"), 
						QString::null, 0, this, 0 );
	actionSendCat->setWhatsThis( tr("Send the visible contacts to another person") );
	connect( actionSendCat, SIGNAL(activated()), this, SLOT(sendContactCat()) );
    }
    else
    {
	delete mSendService;
	mSendService = 0;
	actionSend = 0;
	actionSendCat = 0;
    }
#endif

    actionSettings = new QAction(tr("Settings..."), Resource::loadIconSet("settings"), QString::null, 0, this, 0);
    connect( actionSettings, SIGNAL(activated()), this, SLOT(configure()));

#ifdef AB_PDA
    //FIXME : temporary duplicated icon from contextbar
    actionBack = new QAction(tr("Back"),Resource::loadIconSet("addressbook/back"), QString::null, 0, this, 0);
    connect( actionBack, SIGNAL(activated()), this, SLOT(viewClosed()) );
#endif

#ifndef QTOPIA_PHONE
    // Create Toolbars
    QPEToolBar *bar = new QPEToolBar( this );
    bar->setHorizontalStretchable( TRUE );

    QPEMenuBar *mbList = new QPEMenuBar( bar );
    mbList->setMargin( 0 );

    QPopupMenu *edit = new QPopupMenu( this );
    mbList->insertItem( tr( "Contact" ), edit );

    listTools = new QPEToolBar( this );

    actionNew->addTo( edit );
    actionNew->addTo( listTools );
    actionEdit->addTo( edit );
    actionEdit->addTo( listTools );
    actionTrash->addTo( edit );
    actionTrash->addTo( listTools );
#ifndef QTOPIA_DESKTOP
    if( actionSend ) {
	actionSend->addTo( edit );
	actionSend->addTo( listTools );
	actionSendCat->addTo( edit );
    }
#endif
    actionFind->addTo( edit );
    actionFind->addTo( listTools );
#ifdef AB_PDA
    listTools->addSeparator();
    actionBack->addTo( listTools );
#endif

#else
    contextMenu = new ContextMenu( this );
#ifdef QTOPIA_PHONE
    if( Global::mousePreferred() )
	contextMenu->addTo( mFindLE );
#endif

    actionNew->addTo(contextMenu);
    actionEdit->addTo(contextMenu);
    actionTrash->addTo(contextMenu);
#ifndef QTOPIA_DESKTOP
    if( actionSend ) {
	actionSend->addTo( contextMenu );
	actionSendCat->addTo( contextMenu );
    }
#endif

    actionSetBCard->addTo(contextMenu);

    actionSpeedDial = new QAction(tr("Add to Speed Dial..."),
	Resource::loadIconSet("phone/speeddial"), QString::null, 0, this, 0);
    connect( actionSpeedDial, SIGNAL(activated()), this, SLOT(addToSpeedDial()));
    actionSpeedDial->addTo(contextMenu);

    actionOnSim = new QAction( tr( "Store on SIM" ), QString::null, 0, this, 0 );
    actionOnSim->setToggleAction( TRUE );
    connect( actionOnSim, SIGNAL(toggled(bool)), this, SLOT(storeOnSim(bool)) );
    actionOnSim->addTo( contextMenu );

    actionCategory = new QAction(tr("View Category..."), Resource::loadIconSet("viewcategory"),
			QString::null, 0, this, 0 );
    connect( actionCategory, SIGNAL(activated()), this, SLOT(selectCategory()));
    actionCategory->addTo(contextMenu);

    actionSettings->addTo(contextMenu);

    categoryLbl = new QLabel(listView);
    categoryLbl->hide();

    categoryDlg = 0;
#endif


#ifndef QTOPIA_PHONE

	edit->insertSeparator();
    actionShowBCard->addTo( edit );
    actionSetBCard->addTo( edit );
    //listTools->addSeparator();

    // Filter bar
    QHBox *hb = new QHBox( listView );

    /*
    QComboBox *contactCombo = new QComboBox( hb );
    connect( contactCombo, SIGNAL(activated(int)), this, SLOT(contactFilterSelected(int)) );
    contactCombo->insertItem( PimContact::trFieldsMap()[PimContact::BusinessMobile] );
    contactMap[0]=PimContact::BusinessMobile;
    contactCombo->insertItem( PimContact::trFieldsMap()[PimContact::HomeMobile] );
    contactMap[1]=PimContact::HomeMobile;
    contactCombo->insertItem( PimContact::trFieldsMap()[PimContact::BusinessPhone] );
    contactMap[2]=PimContact::BusinessPhone;
    contactCombo->insertItem( PimContact::trFieldsMap()[PimContact::HomePhone] );
    contactMap[3]=PimContact::HomePhone;
    contactCombo->insertItem( PimContact::trFieldsMap()[PimContact::DefaultEmail] );
    contactMap[4]=PimContact::DefaultEmail;
    contactCombo->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred) );
    contactCombo->setCurrentItem(0);
    contactFilterSelected( 0 );
    QWhatsThis::add( contactCombo, tr("Show this contact information in the Contact column, if available.") );
    */

    cats.load(categoryFileName());
    catSelect = new CategorySelect( hb );
    catSelect->setRemoveCategoryEdit( TRUE );
    connect( catSelect, SIGNAL(signalSelected(int)), this, SLOT(slotSetCategory(int)) );
    QWhatsThis::add( catSelect, tr("Show contacts in this category.") );
    
    QArray<int> vl( 0 );
    catSelect->setCategories( vl, "Address Book", // No tr()
	tr("Address Book") );
    catSelect->setAllCategories( TRUE );
#endif

    abList->setSelectionMode( AbTable::Extended );
//    abList->setFrameStyle( QFrame::NoFrame );
    connect( abList, SIGNAL( clicked() ),
	     this, SLOT( selectClicked() ) );
    connect( abList, SIGNAL(currentChanged()),
	     this, SLOT(updateIcons()) );
    QWhatsThis::add( abList, tr("List of contacts in the selected category.  Click to view detailed information.") );

    readConfig();
    
    setCentralWidget( listView );

#ifndef QTOPIA_PHONE
//    slotSetCategory(-2);
    contacts.setFilter( catSelect->currentCategory() );
    setCaption( tr("Contacts") + " - " + categoryLabel( catSelect->currentCategory() ) );
#else
    setCaption( tr("Contacts") );
#endif
    abList->reload(); // force a reload initially

    constructorDone = TRUE;
    updateIcons();

    connect(qApp, SIGNAL(categoriesChanged()), this, SLOT(catChanged()) );

    // This call is to ensure that all records contains only valid categories 
    catChanged();

    

    connect( qApp, SIGNAL(appMessage(const QCString&,const QByteArray&)), 
	    this, SLOT(appMessage(const QCString&,const QByteArray&)) );
    connect( qApp, SIGNAL(reload()), this, SLOT(reload()) );
    connect( qApp, SIGNAL(flush()), this, SLOT(flush()) );

#ifdef QTOPIA_PHONE
    mPhoneLine = new PhoneLine();
    connect( mPhoneLine->phoneBook(), 
    SIGNAL(entries(const PhoneLine&,const QString&,
	const QValueList<PhoneBookEntry>&)), this, 
    SLOT(phonebookChanged(const PhoneLine&,const QString&,
	    const QValueList<PhoneBookEntry>&)) );
    connect( mPhoneLine, SIGNAL(queryResult(PhoneLine::QueryType,const QString&)), 
	    this, SLOT(setSimFieldLimits(PhoneLine::QueryType,const QString&)) );
    mPhoneLine->phoneBook()->getEntries();
#endif
}

#ifdef QTOPIA_PHONE
bool AddressbookWindow::eventFilter( QObject *o, QEvent *e )
{
    if( Global::mousePreferred() ) {
	if( o == mFindLE && e->type() == QEvent::KeyPress )
	{
	    QKeyEvent *ke = (QKeyEvent *)e;
	    switch( ke->key() )
	    {
		case Key_Select:
		case Key_Left:
		case Key_Right:
		case Key_Up:
		case Key_Down:
		{
		    QPEApplication::postEvent( abList, 
		    new QKeyEvent( QEvent::KeyPress, ke->key(), 0xffff, 0xffff ) );
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}
#endif

void AddressbookWindow::sendContact()
{
    PimContact curEntry;
    if( !abList->hasCurrentEntry() )
	return;
    curEntry = abList->currentEntry();
    QString fileAs = curEntry.fileAs().simplifyWhiteSpace();
    QValueList<PimContact> l; l.append(curEntry);
    sendContacts(fileAs,l);
}

void AddressbookWindow::sendContactCat()
{
    QString lbl = categoryLabel(contacts.filter());
    sendContacts(tr("%1 contacts","eg. business").arg(lbl), abList->all());
}

void AddressbookWindow::sendContacts(const QString& description, const QValueList<PimContact>& list)
{
#ifndef QTOPIA_DESKTOP
    QString filename;
    for( int i = 0 ; i < (int)description.length() ; ++i )
	if( description[i].isLetterOrNumber() || description[i] == '-' )
	    filename += description[i];
	else if( description[i].isSpace() )
	    filename += '-';
    if( filename.isEmpty() )
	filename = "contact";
    filename = QString("%1%2-vcard.vcf").arg(Global::tempDir()).arg( filename );
    PimContact::writeVCard( filename.utf8(), list );
    mSendService->send( filename.utf8(), description );
#endif
}

void AddressbookWindow::setSimFieldLimits( PhoneLine::QueryType 
#ifdef QTOPIA_PHONE
	type
#endif
	, const QString &
#ifdef QTOPIA_PHONE
	value
#endif
	)
{
#ifdef QTOPIA_PHONE
    if( type == PhoneLine::SimFieldLimits )
	AbUtil::setSimFieldLimits( value );
#endif
}

void AddressbookWindow::phonebookChanged( const PhoneLine&,
	const QString& store, const QValueList<PhoneBookEntry>& list )
{
#ifdef QTOPIA_PHONE
    qDebug("phoneBookChanged, num entries =%d", list.count());
    //TODO : check what kind of store this is for?
    if( !mGotSimEntries )
	mPhoneLine->query( PhoneLine::SimFieldLimits ); // first time we've got entries, now find out what the limits are

    if( abEditor )
	abEditor->setHaveSim( TRUE );

    mGotSimEntries = TRUE;
    mPhoneBookEntries = list;
    abList->setSimContacts( AbUtil::phoneBookToContacts( list ) );
    if( mSimIndicator )
	mSimIndicator->hide();
#else
    Q_CONST_UNUSED(list);
#endif
    Q_CONST_UNUSED(store);
}

void AddressbookWindow::keyPressEvent(QKeyEvent *e)
{
#ifdef QTOPIA_PHONE
    if ( (e->key() == Key_Back || e->key() == Key_No)) {
	e->accept();
	if( centralWidget() == listView )
	{
	    allowCloseEvent = TRUE;
	    close();
	}
	else
	{
	    // slotListView();
	    viewClosed(); 
	}
    }
    else
#endif
	QMainWindow::keyPressEvent(e);
}

void AddressbookWindow::showEvent( QShowEvent *e )
{
    QMainWindow::showEvent( e );
#ifdef QTOPIA_PHONE
    if( !mGotSimEntries )
    {
	if( !mSimIndicator )
	{
	    mSimIndicator = new QLabel( tr("Loading SIM..."), listView );
	    mSimIndicator->setAlignment( Qt::AlignCenter );
	}
	mSimIndicator->show();
    }
    if( Global::mousePreferred() )
	mFindLE->setFocus();
#endif
}

void AddressbookWindow::setDocument( const QString &filename )
{
    receiveFile(filename);
}

LoadIndicator::LoadIndicator( QWidget *parent, const char *name, WFlags fl )
    : QLabel( parent, name, fl )
{
    setAlignment( AlignCenter );
    setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
}

void LoadIndicator::center()
{
    QWidget *w = parentWidget();
    if( !w )
	w = qApp->desktop();

    QPoint pp = w->mapToGlobal( QPoint(0,0) ); 
    QSize s = sizeHint() * 2;
    s.setWidth( QMIN(s.width(), w->width()) );
    pp = QPoint( pp.x() + w->width()/2 - s.width()/2,
		pp.y() + w->height()/ 2 - s.height()/2 );

    setGeometry( QRect(pp, s) );
}

void LoadIndicator::bottom()
{
    adjustSize();
    QWidget *w = parentWidget();
    if( !w )
	w = qApp->desktop();
    setGeometry( 0, w->frameGeometry().bottom()-height(), w->width(), height() );
}

void LoadIndicator::showEvent( QShowEvent *e )
{
    QLabel::showEvent( e );
}

void LoadIndicator::paintEvent( QPaintEvent *e )
{
    QPainter p( this );
    p.fillRect( 0, 0, width(), height(), colorGroup().background() );
    p.end();
    QLabel::paintEvent( e );
}

void AddressbookWindow::receiveFile( const QString &filename )
{
    QString targetFile = filename;

    if ( filename.right(8) == ".desktop" ) {
	DocLnk lnk(filename);
	if ( !lnk.isValid() || lnk.type().lower() != "text/x-vcard" )
	    return;
	targetFile = lnk.file();
    } else if ( targetFile.right(4) != ".vcf" ) {
	return;
    }

    QWidget *mbParent = isVisible() ? this : 0;

    LoadIndicator *li = new LoadIndicator( 0 );
    li->setText( tr("Reading VCards..") );
    li->center();
    li->show();
    li->repaint();

    QValueList<PimContact> cl = PimContact::readVCard( targetFile );
    delete li;

    if ( cl.count() == 0 ) {
	QMessageBox::warning(mbParent, tr("Could not read VCard document"),
	    tr("<qt>The VCard document did not contain any valid VCards</qt>") );
	return;
    }

    QValueList<PimContact> newContacts, oldContacts;
    for( QValueList<PimContact>::Iterator it = cl.begin(); it != cl.end(); ++it ) {
	    if ( contacts.contains(*it) ) {
		oldContacts.append( (*it) );
	    } else newContacts.append( (*it) );
    }

    if ( newContacts.count() > 0 ) {
	QString list = "";
	uint count = 0;
	for ( QValueList<PimContact>::Iterator it = newContacts.begin(); it != newContacts.end(); ++it) {
	    if ( count < 3 ) {
		list += (*it).fileAs() + "\n";
	    } else if ( count == 3 ) {
		list += "...\n";
	    }
	    count++;
	}

	QString msg = tr("<qt>%1 new VCard(s) for %2<br>Do you want to add them to your addressbook?</qt>").arg( newContacts.count() ).arg(list);
	if ( QMessageBox::information(mbParent, tr("New contact(s)"), msg, QMessageBox::Ok, QMessageBox::Cancel) ==
	    QMessageBox::Ok )
	{
	    NameLearner namelearner;

	    for ( QValueList<PimContact>::Iterator it = newContacts.begin(); it != newContacts.end(); ++it) 
	    {
		//if we have a photo for this contact, save it to disk
		PimContact &curCon = *it;
		QString encDataStr( curCon.customField( "phototmp" ) );
		if( !encDataStr.isEmpty() )
		{
		    QByteArray encData;
		    encData.duplicate( encDataStr.latin1(), encDataStr.length() );
		    QByteArray decData( Global::decodeBase64( encData ) );

		    QPixmap np( decData );
		    if( !np.isNull() )
		    {
			QString fn( AbEditor::createContactImage( np ) );

			if( fn.isNull() )
			    qWarning(QString("AddressBook::receiveFile - Unable to save photo for contact %1").arg(curCon.fileAs()));
			else
			    curCon.setCustomField( "photofile", fn );
		    }
		    curCon.removeCustomField( "phototmp" );
		}

		namelearner.learn(*it);
		contacts.addContact( *it );
	    }

	    abList->reload();
	}
    } else if ( oldContacts.count() > 0 ) {
	QString list = "";
	uint count = 0;
	for ( QValueList<PimContact>::Iterator it = oldContacts.begin(); it != oldContacts.end(); ++it) {
	    if ( count < 3 ) {
		list += (*it).fileAs() + "\n";
	    } else if ( count == 3 ) {
		list += "...\n";
	    }
	    count++;
	}

	QString msg = tr("<qt>%1 old VCard(s) for %2<br>The document only contained VCards already in your addressbook</qt>").arg( oldContacts.count() ).arg(list);
	QMessageBox::information(mbParent, tr("Contact(s) already registered"), msg, QMessageBox::Ok);

    }
}

void AddressbookWindow::flush()
{
    contacts.saveData();
    syncing = TRUE;
}

void AddressbookWindow::reload()
{
    syncing = FALSE;
#ifndef QTOPIA_PHONE
    QArray<int> vl( 0 );
    catSelect->setCategories( vl, "Address Book", // No tr()
	tr("Address Book") );
    catSelect->setAllCategories( TRUE );
#endif
    //Force a reload here
    abList->reload();
    if ( centralWidget() == mView )
    {
#ifdef QTOPIA_DATA_LINKING
    //just refresh the view
	mView->init( abList->currentEntry() );
	mView->sync();
#else
	slotDetailView();
#endif
    }
    updateIcons();
}

void AddressbookWindow::resizeEvent( QResizeEvent *e )
{
    QMainWindow::resizeEvent( e );

    // we receive a resize event from qtable in the middle of the constrution, since
    // QTable::columnwidth does qApp->processEvents.  Ignore this event as it causes
    // all sorts of init problems for us
    if ( !constructorDone )
	return;

    if ( centralWidget() == listView  )
    {
	slotListView();
    }
    else if ( centralWidget() == mView )
	showView();
}

AddressbookWindow::~AddressbookWindow()
{
    if( contacts.accessMode() != ContactIO::ReadOnly )
	if( !contacts.saveData() )
	    QMessageBox::information( this, tr( "Contacts" ),
		    tr("<qt>Device full.  Some changes may not be saved.</qt>"));
    writeConfig();
    delete abEditor;
#ifdef QTOPIA_PHONE
    delete mPhoneLine;
#endif
}

void
AddressbookWindow::readConfig(void)
{
    Config  config("Contacts");
    config.setGroup("default");
#ifndef QTOPIA_PHONE
    catSelect->setCurrentCategory(config.readNumEntry("Category", -2));
#endif
}

void
AddressbookWindow::writeConfig(void)
{
    Config  config("Contacts");
    config.setGroup("default");

#ifndef QTOPIA_PHONE
    config.writeEntry("Category", catSelect->currentCategory());
#endif
}

void AddressbookWindow::updateIcons()
{
    bool details = centralWidget() == mView;
    actionSettings->setEnabled(!details);
    
#ifdef AB_PDA
    actionBack->setEnabled( details );
#endif

    if ( abList->hasCurrentEntry() ) {
	PimContact ce = abList->currentEntry();

#ifdef QTOPIA_PHONE
	actionSpeedDial->setEnabled( details && (!sel_href.isNull() && !sel_href.startsWith("qdl:")) );
	actionOnSim->setEnabled(details && sel_href.startsWith("dialer:") && mGotSimEntries);
	const bool enable_item_actions = details;
#else
	const bool enable_item_actions = TRUE;
#endif
	actionTrash->setEnabled(enable_item_actions);
	actionEdit->setEnabled(enable_item_actions);
	if (actionFind)
	    actionFind->setEnabled(TRUE);
	if ( abList->selectedContacts().count() == 1 ) {
	    if (actionSetPersonal)
		actionSetPersonal->setEnabled(enable_item_actions);
	} else {
	    if (actionSetPersonal)
		actionSetPersonal->setEnabled(FALSE);
	}
#ifndef QTOPIA_DESKTOP
	if (actionSend) {
#ifdef QTOPIA_PHONE
	    actionSend->setEnabled(details);
#else
	    actionSend->setEnabled(TRUE);
#endif
	    actionSendCat->setEnabled(!details);
	}
#endif
#ifdef QTOPIA_PHONE
	ContextBar::setLabel( abList, Qt::Key_Select, ContextBar::View);
#endif
    } else {
#ifdef QTOPIA_PHONE
	actionSpeedDial->setEnabled(FALSE);
	actionOnSim->setEnabled( FALSE );
#endif
	actionEdit->setEnabled(FALSE);
	actionTrash->setEnabled(FALSE);
	// If find is open, need to close it first.
	if (actionFind && actionFind->isOn())
	    actionFind->setOn(FALSE);
	if (actionFind)
	    actionFind->setEnabled(FALSE);
	if (actionSetPersonal)
	    actionSetPersonal->setEnabled(FALSE);

#ifndef QTOPIA_DESKTOP
	if (actionSend) {
	    actionSend->setEnabled(FALSE);
	    actionSendCat->setEnabled(FALSE);
	}
#endif
#ifdef QTOPIA_PHONE
        ContextBar::setLabel( abList, Qt::Key_Select, "new", QString::null );
#endif
    }

    if (actionPersonal)
	actionPersonal->setEnabled( contacts.hasPersonal() );
}

void AddressbookWindow::showView()
{
    listView->hide();
    setCentralWidget( abView() );
    mView->show();
    mView->setFocus();
#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() )
	mView->setModalEditing( TRUE );
#endif
    updateIcons();
    setCaption( tr("Contact Details") );
#ifdef QTOPIA_PHONE
    actionNew->setEnabled(FALSE);
    actionCategory->setEnabled(FALSE);
#endif
}

bool AddressbookWindow::checkSyncing()
{
    if (syncing) {
	if ( QMessageBox::warning(this, tr("Contacts"),
			     tr("<qt>Can not edit data, currently syncing</qt>"),
			    QMessageBox::Ok, QMessageBox::Abort ) == QMessageBox::Abort )
	{
	    // Okay, if you say so (eg. Qtopia Desktop may have crashed)....
	    syncing = FALSE;
	} else
	    return TRUE;
    }
    return FALSE;
}


void AddressbookWindow::slotListNew()
{
    PimContact cnt;
    if( !checkSyncing() ) {
	abView()->init( cnt );
	newEntry( cnt );
    }
}

void AddressbookWindow::selectClicked()
{
    if( abList->numRows() == 0 )
    {
	slotListNew();
    }
    else
    {
	slotDetailView();
    }
}

void AddressbookWindow::slotDetailView()
{
    if ( abList->hasCurrentEntry() ) {
	if (actionFind && actionFind->isOn())
	    actionFind->setOn(FALSE);
	PimContact curEntry = abList->currentEntry();
	viewOpened( curEntry );
	setHighlightedLink( QString::null );
	abView()->init( curEntry );
	mView->sync();
	showView();
	mResetKeyChars = TRUE;
    }
}

void AddressbookWindow::slotListView()
{
#ifdef QTOPIA_PHONE
    PimContact currentContact = abList->currentEntry();

    if( mResetKeyChars )
    {
	//previous view was something else
	mResetKeyChars = FALSE;
	abList->setKeyChars( "" );
    }
#endif

    if ( !abList->isVisible() ) {
	abView()->hide();
	setCentralWidget( listView );
	listView->show();
#ifdef QTOPIA_PHONE
	if( Global::mousePreferred() )
	    mFindLE->setFocus();
	else
	    abList->setFocus();
#endif

#ifdef QTOPIA_PHONE
	//sim contacts don't have uids, so set it by its fileas
	if( !currentContact.customField("SIM_CONTACT").isEmpty() )
	    abList->setCurrentSimEntry( currentContact.fileAs() );
	else
	    abList->setCurrentEntry( currentContact.uid() );
#endif
	setCaption( tr("Contacts") );
    }
    updateIcons();

#ifdef QTOPIA_PHONE
    actionNew->setEnabled(TRUE);
    actionCategory->setEnabled(TRUE);
#endif
}

void AddressbookWindow::setHighlightedLink(const QString& 
#ifdef QTOPIA_PHONE
	l
#endif
	)
{
#ifdef QTOPIA_PHONE
    sel_href = l;
    //link isn't null and isn't a qdl link
    actionSpeedDial->setEnabled( !l.isNull() && !l.startsWith("qdl:") );
    if( l.startsWith("dialer:") && mGotSimEntries ) 
    {
	actionOnSim->setEnabled( TRUE );
	QString number = l.mid( 7 );
	if( number.startsWith( "phoneType:" ) )
	{
	    number = number.mid( 10 );
	    int f = number.find(":");
	    if( f != -1 )
	    {
		int phoneType = number.left( f ).toInt();
		PimContact ent = mView->entry();
		if( ent.customField("SIM_CONTACT").isEmpty() )
		{
		    //not a sim contact.. merge available phonebook data with the contact.
		    QMap<QString,PimContact *> simData = abList->existingContactSimData();
		    if( simData.contains( ent.fileAs().lower() ) )
			AbUtil::mergeSimContact( ent, *simData[ent.fileAs().lower()] );
		}
		mToggleInternal = TRUE;
		actionOnSim->setOn( ent.customField( AbUtil::simKeyForPhoneType( phoneType ) ).length() );
		mToggleInternal = FALSE;
	    }
	}
    }
    else
    {
	actionOnSim->setEnabled( FALSE );
    }
#endif
}

void AddressbookWindow::storeOnSim( bool 
#ifdef QTOPIA_PHONE
	b 
#endif
	)
{
#ifdef QTOPIA_PHONE
    if( mToggleInternal ) //action was just toggled internally
	return;

    if( sel_href.isNull() )
	return;

    if( sel_href.startsWith("dialer:") )
    {
	QString number = sel_href.mid( 7 );
	if( number.startsWith( "phoneType:" ) )
	{
	    number = number.mid( 10 );
	    int f = number.find(":");
	    if( f != -1 )
	    {
		int phoneType = number.left( f ).toInt();
		number = number.mid( f+1 );
		PimContact ent = mView->entry();
		//toggle the sim key for this number
		bool isSimContact = !ent.customField("SIM_CONTACT").isEmpty();
		if( !isSimContact )
		{
		    //not a sim contact.. merge available phonebook data with the contact.
		    QMap<QString,PimContact *> simData = abList->existingContactSimData();
		    if( simData.contains( ent.fileAs().lower() ) )
			AbUtil::mergeSimContact( ent, *simData[ent.fileAs().lower()] );
		}
		QString k = AbUtil::simKeyForPhoneType( phoneType );
		if( !b )
		    ent.removeCustomField( k );
		else
		    ent.setCustomField( k, "1" );
		bool closeView = FALSE;
		if( isSimContact && AbUtil::numbersForSimCount( ent ) == 0 )
		{
		    //sim contact and all numbers have been removed
		    int r = QMessageBox::warning( this, tr("Remove SIM Contact"), tr("<qt>Removing '%1' will remove the contact '%2' from the SIM entirely. "
								"Are you sure you want to do this?</qt>").arg(number).arg(ent.fileAs()), 
			    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default );
		    if( r == QMessageBox::No )
		    {
			setHighlightedLink( sel_href ); // reset back to on
			return;
		    }
		    else
		    {
			// contact we're viewing will be deleted, so close the view
			closeView = TRUE;
		    }
		}
		AbUtil::syncContactWithSim( ent, mPhoneBookEntries, mPhoneLine->phoneBook() );
		if( closeView )
		    viewClosed();
	    }
	}
    }
#endif
}

void AddressbookWindow::addToSpeedDial()
{
#ifdef QTOPIA_PHONE
    if ( !sel_href.isNull() ) {
	PimContact ent = abList->currentEntry();
	ServiceRequest req;
	QString pm;
	mView->decodeHref(sel_href,&req,&pm);
	SpeedDial::addWithDialog(ent.fileAs(), "addressbook/"+pm, req, this);
    }
#endif
}

void AddressbookWindow::slotListDelete()
{
    if ( checkSyncing() )
	return;

    if ( FALSE ) {
#ifndef QTOPIA_PHONE
	QValueList<QUuid> t = abList->selectedContacts();
	
	if ( !t.count() ) return;
	
	QString str;
	if ( t.count() > 1 ) {
	    str = QString("<qt>Are you sure you want to delete the %1 selected contacts?</qt>").arg( t.count() );
	} else {
	    // some duplicated code, but selected/current logic might change, so I'll leave it as it is
	    PimContact tmpEntry = abList->currentEntry();
	    QString strName = tmpEntry.fileAs();
	    if ( strName.isEmpty() ) {
		strName = tmpEntry.company();
		if ( strName.isEmpty() )
		    strName = tr("No Name");
	    }
	    
	    str = QString("<qt>Are you sure you want to delete: <b>%1</b> ?</qt>").arg( strName );
	}

	switch( QMessageBox::warning( this, tr("Contacts"), tr(str), tr("Yes"), tr("No"), 0, 0, 1 ) ) {
	    case 0:
	    {
		deleteContacts(t);
		updateIcons();
		slotListView();
	    }
	    break;
	    case 1: break;
	}
#endif
    } else {
	PimContact tmpEntry = abList->currentEntry();

	// get a name, do the best we can...
	QString strName = tmpEntry.fileAs();
	if ( strName.isEmpty() ) {
	    strName = tmpEntry.company();
	    if ( strName.isEmpty() )
		strName = tr("No Name");
	}


	if ( QPEMessageBox::confirmDelete( this, tr( "Contacts" ),
					   strName ) ) {
	    PimContact curEntry = abList->currentEntry();

#ifdef QTOPIA_PHONE
	    if( AbUtil::hasNumbersForSim( curEntry ) )
	    {
		for( int i = 0 ; i < AbUtil::numPhoneFields ; ++i )
		    curEntry.setCustomField( 
			AbUtil::simKeyForPhoneType( AbUtil::phoneFields[i] ),
			QString::null );
	    }
	    AbUtil::syncContactWithSim( curEntry, mPhoneBookEntries, mPhoneLine->phoneBook() );
	    if( curEntry.customField( "SIM_CONTACT" ).isEmpty() ) //real contact
#endif
	    contacts.removeContact( curEntry );
	    abList->reload();
	    if( centralWidget() == mView )
		viewClosed();
	    else
		slotListView();
	}
    }
}

void AddressbookWindow::deleteContacts(QValueList<QUuid> &t)
{
    for (QValueList<QUuid>::ConstIterator it = t.begin(); it != t.end(); ++it) {
	PrContact t;
	t.setUid( *it );

        contacts.removeContact(t);
    }
    
    abList->reload();
}


void AddressbookWindow::slotViewBack()
{
    slotListView();
}

void AddressbookWindow::slotViewEdit()
{
    if( !checkSyncing()) {
	editEntry( abList->currentEntry() );
    }
}

void AddressbookWindow::beamContacts(const QString& description, const QValueList<PimContact>& list)
{
    PimContact::writeVCard( beamfile, list );
    beamVCard( beamfile.local8Bit(), description );
}

void AddressbookWindow::beamVCard( const QCString &filename, const QString &description )
{
    unlink( filename.data() ); // delete if exists
    Ir *ir = new Ir( this );
    connect( ir, SIGNAL( done(Ir*) ), this, SLOT( beamDone(Ir*) ) );
    ir->send( filename.data(), description, "text/x-vCard" );

#ifdef DEBUG
    if ( !Ir::supported() ) {
	QMessageBox::information( this, "Beam",
	    tr("<qt>vCard stored in %1</qt>").arg(filename.data()) );
    }
#endif
}

void AddressbookWindow::beamDone( Ir *ir )
{
    delete ir;
   ::unlink( beamfile.local8Bit().data() );
}

/*
static void parseName( const QString& name, QString *first, QString *middle,
		       QString * last )
{

    int comma = name.find ( "," );
    QString rest;
    if ( comma > 0 ) {
	*last = name.left( comma );
	comma++;
	while ( comma < int(name.length()) && name[comma] == ' ' )
	    comma++;
	rest = name.mid( comma );
    } else {
	int space = name.findRev( ' ' );
	*last = name.mid( space+1 );
	rest = name.left( space );
    }
    int space = rest.find( ' ' );
    if ( space <= 0 ) {
	*first = rest;
    } else {
	*first = rest.left( space );
	*middle = rest.mid( space+1 );
    }

}
*/

/*
  Show item defined by uid, then close once user goes back.
*/
void AddressbookWindow::showJustItem(const QUuid& uid)
{
    if( isHidden() ) // only close after view if hidden on first activation 
    {
	mCloseAfterView = TRUE;
    }
#ifdef QTOPIA_PHONE
    abList->setKeyChars( "" );//make the whole list visible
#endif
    mResetKeyChars = FALSE;
    abList->setCurrentEntry( uid );
    slotDetailView();
    updateIcons();
}

void AddressbookWindow::appMessage(const QCString &msg, const QByteArray &data)
{
    if (msg == "editPersonal()") {
	QPEApplication::setKeepRunning();
	editPersonal();
    } else if (msg == "editPersonalAndClose()") {
	QPEApplication::setKeepRunning();
	editPersonal();
	close();
    } else if ( msg == "receiveData(QString,QString)" ) {
	QDataStream stream(data,IO_ReadOnly);
	QString f,t;
	stream >> f >> t;
	if ( t.lower() == "text/x-vcard" )
	    receiveFile(f);
	QFile::remove(f);

    } else if ( msg == "addContact(PimContact)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimContact c;
	stream >> c;
	NameLearner learner(c);
	contacts.addContact( c );
	abList->reload();

    } else if ( msg == "removeContact(PimContact)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimContact c;
	stream >> c;
        contacts.removeContact( c );
	abList->reload();
    } else if ( msg == "updateContact(PimContact)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimContact c;
	stream >> c;
	NameLearner learner(c);
	contacts.updateContact( c );
	abList->reload();
    } else if ( msg == "addAndEditContact(PimContact)" ) {
	QPEApplication::setKeepRunning();
	QDataStream stream(data,IO_ReadOnly);
	PimContact c;
	stream >> c;

	abView()->init( c );
	newEntry( c );
    } else if ( msg == "showContact(QUuid)" ) {
	QPEApplication::setKeepRunning();
	QUuid uid;
	QDataStream stream(data, IO_ReadOnly);
	stream >> uid;
	showJustItem(uid);
#ifdef Q_WS_QWS
    } else if ( msg == "beamBusinessCard()" ) {
	if (contacts.hasPersonal()) {
	    QString description;
	    ::unlink( beamfile.local8Bit().data() ); // delete if exists
	    PimContact c((const PimContact &)contacts.personal());
	    PimContact::writeVCard( beamfile, c );
	    description = c.fileAs();
	    Ir *ir = new Ir( this );
	    connect( ir, SIGNAL( done(Ir*) ), this, SLOT( beamDone(Ir*) ) );
	    ir->send( beamfile, description, "text/x-vCard" );
	}
	else
	{
	    QMessageBox::warning( this, tr("Contacts"), "<qt>"+tr("No business card set.")+"</qt>");
	}
#endif
    }
#ifdef QTOPIA_PHONE
    else if( msg == "smsBusinessCard()")
    {
	if( abList->numRows() == 0 || !contacts.hasPersonal()) {
	    QMessageBox::warning( this, tr("Contacts"), "<qt>"+tr("No business card set.")+"</qt");
	} 
	else 
	{
	    //QPEApplication::setKeepRunning();
	    smsBusinessCard();
	}
    }
#endif
    else if( msg == "addPhoneNumberToContact(QString)" )
    {
	QDataStream stream( data, IO_ReadOnly );
	QString phoneNumber;
	stream >> phoneNumber;

	ContactSelector *s = new ContactSelector( TRUE, &contacts, this );
	if(
#ifndef QTOPIA_DESKTOP
	    QPEApplication::execDialog( s ) 
#else
	    s->exec()
#endif
	    && s->selectedType() != ContactSelector::NoSelection 
	  )
	{
	    PimContact cnt;
	    bool isNew = FALSE;
	    if( s->selectedType() == ContactSelector::SelectedContacts )
	    {
		QValueList<PimContact> selCnts = s->selected();
		if( selCnts.count() <= 0)
		    qDebug("BUG - Selected contacts is 0 but we have a selection");
		cnt = selCnts.first();
	    }
	    else if( s->selectedType() == ContactSelector::SelectedNew )
	    {
		isNew = TRUE;
	    }

	    //get the user to choose the type of the number
	    PhoneTypeSelector *pts = new PhoneTypeSelector( cnt, phoneNumber, this );
	    if(
#ifndef QTOPIA_DESKTOP
	    QPEApplication::execDialog( pts ) 
#else
	    pts->exec()
#endif
	    && pts->selected() != -1
	      )
	    {
		int f = pts->selected();
		cnt.setField( f, phoneNumber );

		if( isNew )
		{
		    abView()->init( cnt );
		    newEntry( cnt );
		}
		else
		{
		    contacts.updateContact( cnt );
		    abList->reload();
		}
	    }
	}
    }
#ifdef QTOPIA_DATA_LINKING
    else if( msg == "QDLRequestLink(QString,QString)" )
    {
	QDataStream stream( data, IO_ReadOnly );
	QString clientID, hint;
	stream >> clientID >> hint;
	QDLHeartBeat hb( clientID );

	ContactSelector *s = new ContactSelector( FALSE, &contacts,
	    ( isVisible() ? this : 0 ), "selector", TRUE ); // No tr
#ifndef QTOPIA_DESKTOP
	if( QPEApplication::execDialog( s ) == QDialog::Accepted )
#else
	if( s->exec() == QDialog::Accepted )
#endif
	{
#ifndef QT_NO_QCOP
	    QCopEnvelope e( QDL::CLIENT_CHANNEL, "QDLProvideLink(QString,int,...)" );

	    e << clientID;

	    QValueList<PimContact> contacts = s->selected();
	    e << contacts.count();

	    QValueList<PimContact>::Iterator it;
	    for( it = contacts.begin() ; it != contacts.end() ; ++it )
	    {
		QByteArray dataRef;
		QDataStream refStream( dataRef, IO_WriteOnly );
		refStream << (*it).uid().toString();
		e << QDLLink( "addressbook",  dataRef, (*it).fileAs(), "addressbook/AddressBook" );
	    }
#endif
	}
	delete s;
    }
    else if( msg == "QDLActivateLink(QByteArray)" )
    {
	QPEApplication::setKeepRunning();
	QDataStream stream( data, IO_ReadOnly );
	QByteArray dataRef;
	stream >> dataRef;
	QDataStream refStream( dataRef, IO_ReadOnly );
	QString uidStr;
	refStream >> uidStr;
	QUuid uid( uidStr );
	showJustItem(uid);
    }
#endif
    else if( msg == "setContactImage(QImage)" )
    {
	QDataStream stream( data, IO_ReadOnly );
	QImage img;
	stream >> img;
	ContactSelector *s = new ContactSelector( FALSE, &contacts, this );
	s->table->setSelectionMode( AbTable::Single );
	if(
#ifndef QTOPIA_DESKTOP
	    QPEApplication::execDialog( s ) 
#else
	    s->exec()
#endif
	    && s->selectedType() == ContactSelector::SelectedContacts
	  )
	{
	    PimContact cnt = s->selected().first();
	    QString baseDirStr = Global::applicationFileName( "addressbook", 
							    "contactimages/" );
	    QString pFileName;
	    //remove an old contact image if present
	    pFileName = cnt.customField( "photofile" );
	    if( !pFileName.isEmpty() )
	    {
		QFile oldPhoto( baseDirStr+pFileName );
		oldPhoto.remove();
	    }
	    //create new image
	    QPixmap np;
	    np.convertFromImage( img );
	    pFileName = AbEditor::createContactImage( np );
	    if( pFileName.isEmpty() )
		qWarning("Unable to save photo file for contact %s", 
			cnt.fileAs().latin1());
	    cnt.setCustomField( "photofile", pFileName );
	    contacts.updateContact( cnt );
	    abList->reload();
	}
    }
    else if( msg == "beamVCard(QCString,QString,QCString,QString)" )
    {
	QPEApplication::setKeepRunning();
	QCString rc, vc;
	QString id, description;
	QDataStream stream( data, IO_ReadOnly );
	stream >> rc >> id >> vc >> description;
	beamVCard( vc, description );
    }
}

void AddressbookWindow::slotPersonalView()
{
    if (showingPersonal) {
	// we just turned it off
	setCaption( tr("Contacts") );
	updateIcons();
	slotListView();
	showingPersonal = FALSE;
	return;
    }

    setCaption( tr("Contacts - My Personal Details") );

    PimContact me((const PimContact &)contacts.personal());
    abList->setCurrentEntry( me.uid() );
    
    viewOpened( me );
    abView()->init( me );
    abView()->sync();
    listView->hide();
    setCentralWidget( abView() );
    mView->show();
    mView->setFocus();
    updateIcons();
}

// hacky static bool
static bool newPersonal = FALSE;

void AddressbookWindow::editPersonal()
{
    if (contacts.hasPersonal()) {
	PrContact me(contacts.personal());
	abView()->init( me );
	newPersonal = FALSE;
	editEntry( me );
    } else {
	newPersonal = TRUE;
	newEntry();
    }
    newPersonal = FALSE;
}

void AddressbookWindow::markCurrentAsPersonal()
{
    if ( !abList->hasCurrentEntry() ) {
	qWarning("ab::markCurrentAsPersonal called with no selection");
	return;
    }

    if ( abList->selectedContacts().count() == 1 ) {
	PimContact c = abList->currentEntry();
	if (QMessageBox::information(this, tr("Contacts"),
		tr("<qt>Set \"%1\" as your Business Card?</qt>").arg( c.fileAs() ),
		tr("Yes"), tr("No"), 0, 0, 1) == 0) {
	    contacts.setAsPersonal(c.uid());
        abList->reload();
	    updateIcons();
	}
    }
}

void AddressbookWindow::viewNext()
{
#ifndef QTOPIA_PHONE
    if ( abList->hasCurrentEntry() ) {
	int cr = abList->currentRow();
	if ( ++cr < abList->numRows() ) {
	    abList->setCurrentCell( cr, 0 );
	    abView()->init( abList->currentEntry() );
	    mView->sync();
	}
    }
#endif
}

void AddressbookWindow::viewPrevious()
{
#ifndef QTOPIA_PHONE
    if ( abList->hasCurrentEntry() ) {
	int cr = abList->currentRow();
	if ( --cr >= 0 ) {
	    abList->setCurrentCell( cr, 0 );
	    abView()->init( abList->currentEntry() );
	    mView->sync();
	}
    }
#endif
}

void AddressbookWindow::newEntry()
{
    newEntry( PimContact() );
}

void AddressbookWindow::newEntry( const PimContact &cnt )
{
    PimContact entry = cnt;
    AbEditor *ed = editor();
#ifndef QTOPIA_PHONE
    QArray<int> cats(1);
    cats[0] = catSelect->currentCategory();
    entry.setCategories( cats );
#else
    if( contacts.filter() != -1 && contacts.filter() != -2 )
    {
	QArray<int> cats(1);
	cats[0] = contacts.filter();
	entry.setCategories( cats );
    }
#endif
    ed->setEntry( entry, TRUE );
#ifdef QTOPIA_PHONE
    ed->setHaveSim( mGotSimEntries );
#endif

    if ( QPEApplication::execDialog(ed) == QDialog::Accepted ) 
    {
	setFocus();
	PimContact newEntry = ed->entry();
#ifdef QTOPIA_PHONE
	AbUtil::syncContactWithSim( newEntry, mPhoneBookEntries, mPhoneLine->phoneBook() );
	//don't store sim keys in the database
	for( int i = 0 ; i < AbUtil::numPhoneFields ; ++i )
	    newEntry.setCustomField( AbUtil::simKeyForPhoneType( AbUtil::phoneFields[i] ),
							    QString::null ); 
#endif
	NameLearner learner(newEntry);
	QUuid ui = contacts.addContact( newEntry );
	if (newPersonal)
	    contacts.setAsPersonal(ui);

#ifdef QTOPIA_PHONE
	abList->setKeyChars("");
#endif
	abList->reload();
	updateIcons();
	abList->setCurrentEntry(ui);
    }
}

AbEditor *AddressbookWindow::editor()
{
    if( !abEditor )
	abEditor = new AbEditor( isVisible() ? this : 0, "edit" );
    return abEditor;
}

void AddressbookWindow::editEntry( const PimContact &cnt )
{
    PimContact entry = cnt;
    AbEditor *ed = editor();
#ifdef QTOPIA_PHONE
    QMap<QString,PimContact *> simData = abList->existingContactSimData();
    QString entryFileAs = entry.fileAs().lower();
    if( simData.contains( entryFileAs ) )
	AbUtil::mergeSimContact( entry, *simData[entryFileAs] );
#endif
    ed->setNameFocus();
    ed->setEntry( entry );

#ifdef QTOPIA_PHONE
    ed->setHaveSim( mGotSimEntries );

    bool isSimContact = !entry.customField( "SIM_CONTACT" ).isEmpty();
    bool simContactRemoved = FALSE;
#endif
    if ( QPEApplication::execDialog(ed) ) 
    {
	setFocus();
	PimContact editedEntry( ed->entry() );
	QUuid entryUid = editedEntry.uid();
	if( !AbUtil::compareContacts( entry, editedEntry ) || ed->imageModified() ) 
	{// only do update operations if the data has actually changed

	    QPixmap *cached = QPixmapCache::find( "pimcontact" + entryUid.toString() + "-cfl" );
	    if( ed->imageModified() && cached )
	    {
		//update the contact field list image cache
		QString photoFile( editedEntry.customField( "photofile" ) );
		QPixmap p;
		if( !photoFile.isEmpty() )
		{
		    QString baseDirStr = Global::applicationFileName( "addressbook", "contactimages/" );
		    QString pFileName( baseDirStr + photoFile  );
		    p.convertFromImage(Image::loadPortrait(pFileName, AbUtil::ImageWidth, AbUtil::ImageHeight));
		}
		QPixmapCache::insert( "pimcontact" + entryUid.toString() + "-cfl", p );
	    }
#ifdef QTOPIA_PHONE
	    //if the name has changed, remove all numbers under the old name from the sim
	    if( entry.fileAs().lower() != editedEntry.fileAs().lower() )
	    {
		PimContact tmp;
		tmp.setFileAs( entry.fileAs() );
		AbUtil::syncContactWithSim( tmp, mPhoneBookEntries, mPhoneLine->phoneBook() );
	    }
	    AbUtil::syncContactWithSim( editedEntry, mPhoneBookEntries, mPhoneLine->phoneBook() );

#endif

#ifdef QTOPIA_PHONE
	    bool hasNumbersForSim = AbUtil::hasNumbersForSim( editedEntry );

	    // don't store sim keys in the database
	    AbUtil::unmergeSimContact( editedEntry );

	    if( isSimContact )
	    {
		//sim contact
		bool hasNonSimFields = ed->hasNonSimFields();
		if( !hasNumbersForSim && !hasNonSimFields )
		{
		    //no fields for sim and no other data, so the contact won't be around on the next reload
		    simContactRemoved = TRUE;
		}
		else if( hasNonSimFields )
		{
		    //has non sim related data, so add it as a real contact
		    editedEntry.setCustomField( "SIM_CONTACT", QString::null );
		    NameLearner learner(editedEntry);
		    entryUid = contacts.addContact( editedEntry );
		    isSimContact = FALSE;
		}
	    }
	    else
	    {
#endif
		//regular contact, just update
		NameLearner learner(editedEntry);
		contacts.updateContact( editedEntry );
#ifdef QTOPIA_PHONE
	    }
#endif
	    abList->reload();
#ifdef QTOPIA_PHONE
	    //reset the current entry to the entry just edited
	    if( isSimContact && !simContactRemoved ) // sim contact, not removed
		abList->setCurrentSimEntry( editedEntry.fileAs() );
	    else if( isSimContact && simContactRemoved )
		; // sim contact removed - nothing to set
	    else if( !entryUid.isNull() )
#endif
	    {
		// not a sim contact, set current normally
		abList->setCurrentEntry( entryUid );
	    }

#ifdef QTOPIA_PHONE
	    if( (isSimContact && simContactRemoved) )
		viewClosed(); // can't go back to viewing a removed sim contact
	    else
	    {
#endif
		if( centralWidget() == mView )
		{
		    // don't call slotDetailView because with QDL that would push the same
		    // entry onto the view stack again. just refresh
		    mView->init( editedEntry );
		    mView->sync();
		}
#ifdef QTOPIA_PHONE
	    }
#endif
	}
    }
}

void AddressbookWindow::closeEvent( QCloseEvent *e )
{
    //don't ignore a closeEvent - trap closing before we get to this point on QTOPIA_PHONE
#ifdef QTOPIA_PHONE
	if( mSimIndicator )
	    mSimIndicator->hide();
#endif
#ifdef QTOPIA_DATA_LINKING
#ifndef AB_PDA
	if( !allowCloseEvent )
	{
	    e->ignore();
	    return;
	}
#endif
#endif
	slotListView();
	QMainWindow::closeEvent( e );
}


AbLabel *AddressbookWindow::abView()
{
    if ( !mView ) {
      mView = new AbLabel( this );
      connect( mView, SIGNAL(externalLinkActivated()), this, SLOT(close()) );
      connect( mView, SIGNAL(highlighted(const QString&)),
	this, SLOT(setHighlightedLink(const QString&)) );
      mView->setMargin(0);
      setHighlightedLink(QString::null);
      mView->init( PimContact()  );
      //connect( mView, SIGNAL(okPressed()), this, SLOT( viewClosed() ) );
      //connect( mView, SIGNAL(previous()), this, SLOT( viewPrevious() ) );
      //connect( mView, SIGNAL(next()), this, SLOT( viewNext() ) );
#ifdef QTOPIA_PHONE
      if( Global::mousePreferred() ) 
	  mView->setFocusPolicy( NoFocus );
#endif
    }
    return mView;
}

void AddressbookWindow::viewOpened( const PimContact &entry )
{
#ifdef QTOPIA_DATA_LINKING
	PimContact last;
	if( !mContactViewStack.count() || 
		(((last=mContactViewStack.last()).customField("SIM_CONTACT").isEmpty() 
		     &&  mContactViewStack.last().uid() != entry.uid() )) ||
		(last.customField("SIM_CONTACT").length() && !AbUtil::compareContacts(last, entry)) )// compare data of sim contacts
	    mContactViewStack.append( entry );
#endif
}

void AddressbookWindow::viewClosed()
{
#ifdef QTOPIA_DATA_LINKING
    if( mContactViewStack.count() )
    {
	QValueList<PimContact>::Iterator it = mContactViewStack.fromLast();
	mContactViewStack.remove( it );
	if( !mContactViewStack.count() )
	{
	    if( mCloseAfterView )
	    {
		allowCloseEvent = TRUE;
                mCloseAfterView = FALSE;
                slotListView();
		close();
		mCloseAfterView = FALSE;
	    }
	    else
	{
		allowCloseEvent = FALSE;
		slotListView();
	}
	}
	else
	{
	    allowCloseEvent = FALSE;
	    PimContact prevContact = mContactViewStack.last();
	    abList->setCurrentEntry( prevContact.uid() );
	    slotDetailView();
	}
    }
    else 
#endif
    if( mCloseAfterView )
    {
        mCloseAfterView = FALSE;
	allowCloseEvent = TRUE;
        slotListView();
	close();
    }
    else
    {
	allowCloseEvent = FALSE;
	slotListView();
    }
}

void AddressbookWindow::slotFind(bool s)
{
#ifndef QTOPIA_PHONE
    if ( !searchBar ) {
	// Search bar
	searchBar = new QPEToolBar(this);
	addToolBar( searchBar, "", QMainWindow::Top, TRUE );

	searchBar->setHorizontalStretchable( TRUE );

	searchEdit = new QLineEdit( searchBar, "searchEdit" );
	QPEApplication::setInputMethodHint(searchEdit,"addressbook-names");
	searchBar->setStretchableWidget( searchEdit );
	connect( searchEdit, SIGNAL(textChanged(const QString&)),
		this, SLOT(search(const QString &)) );
    }
    if ( s ) {
	if ( centralWidget() == abView() )
	    slotListView();
	searchBar->show();
	searchEdit->setFocus();
	abList->setKeyChars( searchEdit->text() );
    } else {
	searchBar->hide();
	if ( abList->numSelections() )
	    abList->clearSelection();
	bool hasCurrentEntry = abList->hasCurrentEntry();
	QUuid uid;
	if( hasCurrentEntry )
	    uid = abList->currentEntry().uid();
	abList->setKeyChars(""); // destroys current
	abList->setFocus();
	if( hasCurrentEntry )
	    abList->setCurrentEntry( uid );
    }
#else
    Q_UNUSED(s);
#endif
}

void AddressbookWindow::search( const QString &k )
{
#ifndef QTOPIA_DESKTOP
    abList->setKeyChars( k );
#endif
}

void AddressbookWindow::slotSetCategory( int c )
{
    //abList->setPaintingEnabled( false );
    contacts.setFilter( c );
    abList->reload();
    //abList->setPaintingEnabled( true );
#ifndef QTOPIA_DESKTOP
#ifndef QTOPIA_PHONE
    setCaption( tr("Contacts") + " - " + categoryLabel( c ) );
#else
    if (c == -2) {
	if ( actionSendCat )
	    actionSendCat->setText(tr("Send All..."));
	categoryLbl->hide();
    } else {
	QString lbl = categoryLabel(c);
	if ( actionSendCat )
	    actionSendCat->setText(tr("Send %1...").arg(lbl));
	categoryLbl->setText(tr("Category: %1").arg(lbl));
	categoryLbl->show();
    }
#endif
#endif
}

// Loop through and validate the categories.  If any records' category list was
// modified we need to update
void AddressbookWindow::catChanged()
{
    Categories c;
    c.load( categoryFileName() );
    QArray<int> cats = c.ids("Address Book", c.labels("Address Book", TRUE) ); // No tr
    bool changed = FALSE;

    QListIterator<PrContact> it(contacts.contacts());
    for(; it.current(); ++it) 
    {
	PimContact t( *(*it) );
	if ( t.pruneDeadCategories( cats ) ) 
	{
	    contacts.updateContact( t );
	    changed = TRUE;
	}
    }

    
    if ( changed )
	abList->reload();
}


QString AddressbookWindow::categoryLabel( int id )
{
    // This is called seldom, so calling a load in here
    // should be fine.
    cats.load( categoryFileName() );
    if ( id == -1 )
	return tr( "Unfiled" );
    else if ( id == -2 )
	return tr( "All" );
    
    QArray<int> ids( 1 );
    ids[0] = id;
    return cats.displaySingle( "Address Book", // No tr()
		    ids, Categories::ShowFirst );
}

/* simply disabled for now although this might be useful
void AddressbookWindow::contactFilterSelected( int idx )
{
    int cid = contactMap[idx];
    abList->setPreferredField( cid );
    qDebug("reloading with preferred");
    abList->reload();
}
*/

void AddressbookWindow::selectAll()
{
//    abList->selectAll();
//    updateIcons();
}

void AddressbookWindow::configure()
{
    AbSettings settings(this, "", TRUE);
#ifndef QTOPIA_PHONE
    settings.setCurrentFields( abList->fields() );
#endif
    if ( QPEApplication::execDialog(&settings) == QDialog::Accepted ) {
#ifndef QTOPIA_PHONE
	abList->setFields( settings.fields() );
	abList->reload();
#else
	settings.saveFormat();

	// Could try to preserve special-cased fileAs items,
	// but the semantics for those needs to be defined.
	// Could ask the user, but it's not the common case.

	bool changed = FALSE;
	extern void qpe_setNameFormatCache(bool);
	qpe_setNameFormatCache(TRUE);
	QListIterator<PrContact> it(contacts.contacts());
	for(; it.current(); ++it)
	{
	    PimContact t( *(*it) );
	    QString fn = t.fullName();
	    if ( fn != t.fileAs() ) {
		t.setFileAs(fn);
		contacts.updateContact( t );
		changed = TRUE;
	    }
	}
	qpe_setNameFormatCache(FALSE);
	if ( changed )
	    abList->reload();
#endif
    }
}

void AddressbookWindow::selectCategory()
{
#ifdef QTOPIA_PHONE
    // always recreate so it gets refreshed properly
    categoryDlg = new CategorySelectDialog("Address Book", this, 0, TRUE);
    categoryDlg->setAllCategories(TRUE);
    connect(categoryDlg, SIGNAL(selected(int)), this, SLOT(slotSetCategory(int)));
    QPEApplication::execDialog(categoryDlg);
    delete categoryDlg;
    categoryDlg = 0;
#endif
}

#ifdef QTOPIA_PHONE
void AddressbookWindow::smsBusinessCard()
{
    bool srvExists = FALSE; // check for SMS service
    Config srv(Service::config("Email"),Config::File);
    srvExists = srv.isValid();
    if ( !abList->hasCurrentEntry() || !srvExists) // nothing selected or srv not avail
    {
	    close();
	    return;
    }
    
    QString description;
    ::unlink( beamfile.local8Bit().data() ); // delete if exists

    // 160 chars limitation for a sms
    // business numbers and home mobile will be transmitted
    PimContact my((const PimContact &)contacts.personal());
    PimContact bCard;
    bCard.setFirstName(my.firstName());
    bCard.setLastName(my.lastName());	
    bCard.setBusinessPhone(my.businessPhone());
    bCard.setBusinessFax(my.businessFax());
    bCard.setBusinessMobile(my.businessMobile());
    bCard.setHomeMobile(my.homeMobile());
    PimContact::writeVCard( beamfile, bCard);
    
    ServiceRequest service("Email", "writeSms(QString,QString,QCString)");
    service<< QString::null;
    service<< QString::null;
    service<< QCString(beamfile);
    service.send();
    close();
}
#endif

