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

#define QTOPIA_INTERNAL_FD

#include "abeditor.h"
#include "ablabel.h"
#include "abtable.h"
#include "addressbook.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/pim/contact.h>
#include <qtopia/global.h>
#include <qtopia/resource.h>
#include <qtopia/ir.h>
#include <qtopia/qpemessagebox.h>
#include <qtopia/categoryselect.h>
#include <qtopia/categories.h>
#include <qtopia/applnk.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/services.h>
#include <qtopia/qpemenubar.h>
#include <qtopia/qpetoolbar.h>

#include <qaction.h>
#include <qdialog.h>
#include <qdir.h>
#include <qfile.h>
#include <qimage.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qwhatsthis.h>

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#ifndef Q_OS_WIN32
#   include <unistd.h>
#endif

#include <qdatetime.h>

static bool constructorDone = FALSE;

AbSettings::AbSettings(QWidget *parent, const char *name, bool modal, WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setCaption(tr("Configure columns to display"));

    QGridLayout *grid = new QGridLayout(this);
    map = new FieldMap(this);
    grid->addWidget(map, 0, 0);
}

void AbSettings::setCurrentFields(const QValueList<int> &f)
{
    QMap<int,QString> m = PimContact::trFieldsMap();
    m.remove( PimRecord::Categories );
    m.insert( AbTable::FREQ_CONTACT_FIELD, tr("Contact") );
    map->setFields( m, f);
}

AddressbookWindow::AddressbookWindow( QWidget *parent, const char *name,
				      WFlags f )
    : QMainWindow( parent, name, f ),
      contacts( ContactIO::ReadWrite ),
      abEditor(0),
      mView(0),
      bAbEditFirstTime(TRUE),
      syncing(FALSE),
      showingPersonal(FALSE)
{
    setCaption( tr("Contacts") );
    setIcon( Resource::loadPixmap( "AddressBook" ) );

    setToolBarsMovable( FALSE );
    setBackgroundMode( PaletteButton );

    // Create Toolbars

    QPEToolBar *bar = new QPEToolBar( this );
    bar->setHorizontalStretchable( TRUE );

    QPEMenuBar *mbList = new QPEMenuBar( bar );
    mbList->setMargin( 0 );

    QPopupMenu *edit = new QPopupMenu( this );
    mbList->insertItem( tr( "Contact" ), edit );

    viewmenu = new QPopupMenu( this );
    mbList->insertItem( tr( "View" ), viewmenu );
    
    viewmenu->insertItem( tr("Select All"), this, SLOT( selectAll() ) );

    viewmenu->insertSeparator();
    viewmenu->insertItem( tr("Configure headers"), this, SLOT(configure()) );
    listTools = new QPEToolBar( this );


    QAction *a = new QAction( tr( "New" ), Resource::loadIconSet( "new" ), QString::null,
			      0, this, 0 );
    actionNew = a;
    connect( a, SIGNAL( activated() ), this, SLOT( slotListNew() ) );
    a->setWhatsThis( tr("Enter a new contact.") );
    a->addTo( edit );
    a->addTo( listTools );

    a = new QAction( tr( "List" ), Resource::loadIconSet( "list" ), QString::null,
			      0, this, 0 );
    actionList = a;
    connect( a, SIGNAL( activated() ), this, SLOT( slotListView() ) );
    a->setWhatsThis( tr("View list of Contacts.") );
    a->addTo( edit );
    a->addTo( listTools );
    a->setEnabled( FALSE );

    a = new QAction( tr( "Edit" ), Resource::loadIconSet( "edit" ), QString::null,
				  0, this, 0 );
    actionEdit = a;
    a->setWhatsThis( tr("Edit the highlighted contact.") );
    connect( a, SIGNAL( activated() ), this, SLOT( slotViewEdit() ) );
    a->setEnabled(FALSE);
    a->addTo( edit );
    a->addTo( listTools );

    a = new QAction( tr( "Duplicate" ), Resource::loadIconSet( "copy" ), QString::null,
				  0, this, 0 );
    actionDuplicate = a;
    a->setWhatsThis( tr("Duplicate the highlighted contact.") );
    connect( a, SIGNAL( activated() ), this, SLOT( slotViewDuplicate() ) );
    a->setEnabled(FALSE);
    a->addTo( edit );

    a = new QAction( tr( "Delete" ), Resource::loadIconSet( "trash" ), QString::null,
		     0, this, 0 );
    actionTrash = a;
    a->setWhatsThis( tr("Delete the selected contacts.") );
    connect( a, SIGNAL( activated() ), this, SLOT( slotListDelete() ) );
    a->setEnabled(FALSE);
    a->addTo( edit );
    a->addTo( listTools );

    a = new QAction( tr( "Find" ), Resource::loadIconSet( "find" ),
                     QString::null, 0, this, 0 );
    actionFind = a;
    a->setWhatsThis( tr("Find a contact.") );
    a->setToggleAction( TRUE );
    connect( a, SIGNAL(toggled(bool)), this, SLOT(slotFind(bool)) );
    a->addTo( edit );
    a->addTo( listTools );

    a = new QAction( tr( "Write Mail To" ), Resource::loadIconSet( "qtmail/reply" ),
                     QString::null, 0, this, 0 );
    a->setEnabled( FALSE );
    a->setWhatsThis( tr("Write an email to this contact.") );
    actionMail = a;
    connect( a, SIGNAL( activated() ), this, SLOT( writeMail() ) );
    a->addTo( edit );
    a->addTo( listTools );

    if ( Ir::supported() ) {
	a = new QAction( tr ("Beam Entry" ), Resource::loadIconSet( "beam" ),
	    QString::null, 0, this, 0 );
	actionBeam = a;
	connect( a, SIGNAL( activated() ), this, SLOT( slotBeam() ) );
	a->setWhatsThis( tr("Beam the selected contacts to another device.") );
	a->addTo( edit );
	a->addTo( listTools );
    }else
      actionBeam = 0L;

    edit->insertSeparator();

    a = new QAction( tr("Show Business Card"), QString::null, 0, 0, 0, FALSE );
    actionPersonal = a;
    a->setWhatsThis( tr("Show your business card entry.") );
    connect( a, SIGNAL( activated() ), this, SLOT( slotPersonalView() ) );
    a->addTo( edit );

    a = new QAction( tr("Set as Business Card"), QString::null, 0, 0, 0, FALSE );
    actionSetPersonal = a;
    a->setWhatsThis( tr("Set current entry as your business card.") );
    connect( a, SIGNAL( activated() ), this, SLOT( markCurrentAsPersonal() ) );
    a->addTo( edit );


    // Search bar
    searchBar = new QPEToolBar(this);
    addToolBar( searchBar, "", QMainWindow::Top, TRUE );

    searchBar->setHorizontalStretchable( TRUE );

    searchEdit = new QLineEdit( searchBar, "searchEdit" );
    searchBar->setStretchableWidget( searchEdit );
    connect( searchEdit, SIGNAL(textChanged(const QString &)),
	    this, SLOT(search()) );
    connect( searchEdit, SIGNAL(returnPressed()), this, SLOT(search()) );

    a = new QAction( tr( "Find Next" ), Resource::loadIconSet( "next" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL(activated()), this, SLOT(search()) );
    a->setWhatsThis( tr("Find the next occurrence of the search text.") );
    a->addTo( searchBar );

    searchBar->hide();

    // Create Views

    listView = new QVBox( this );

    abList = new AbTable( contacts.sortedContacts(), listView );
    
    // can create this item now
    viewmenu->insertItem( tr("Fit to width"), abList, SLOT(fitHeadersToWidth()) );
    
    abList->setSelectionMode( AbTable::Extended );
//    abList->setFrameStyle( QFrame::NoFrame );
    connect( abList, SIGNAL( clicked() ),
	     this, SLOT( slotDetailView() ) );
    connect( abList, SIGNAL(currentChanged(int,int)),
	     this, SLOT(updateIcons()) );
    connect( abList, SIGNAL(findWrapAround()), this, SLOT(findWrapped()) );
    connect( abList, SIGNAL(findNotFound()), this, SLOT(findNotFound()) );
    connect( abList, SIGNAL(findFound()), this, SLOT(findFound()) );
    QWhatsThis::add( abList, tr("List of contacts in the selected category.  Click to view detailed information.") );

    // Filter bar
    QHBox *hb = new QHBox( listView );

    QComboBox *contactCombo = new QComboBox( hb );
    connect( contactCombo, SIGNAL(activated(int)), this, SLOT(contactFilterSelected(int)) );
    contactCombo->insertItem( tr("Email") );
    contactMap[0]=PimContact::DefaultEmail;
    contactCombo->insertItem( tr("Business Phone") );
    contactMap[1]=PimContact::BusinessPhone;
    contactCombo->insertItem( tr("Home Phone") );
    contactMap[2]=PimContact::HomePhone;
    contactCombo->setCurrentItem(0);
    contactFilterSelected( 0 );
    QWhatsThis::add( contactCombo, tr("Show this contact information in the Contact column, if available.") );

    cats.load(categoryFileName());
    catSelect = new CategorySelect( hb );
    catSelect->setRemoveCategoryEdit( TRUE );
    connect( catSelect, SIGNAL(signalSelected(int)), this, SLOT(slotSetCategory(int)) );
    QWhatsThis::add( catSelect, tr("Show contacts in this category.") );
    
    QArray<int> vl( 0 );
    catSelect->setCategories( vl, "Address Book", // No tr()
	tr("Address Book") );
    catSelect->setAllCategories( TRUE );

    readConfig();
    
    setCentralWidget( listView );

//    slotSetCategory(-2);
    contacts.setFilter( catSelect->currentCategory() );
    abList->reload( contacts.sortedContacts() );
    setCaption( tr("Contacts") + " - " + categoryLabel( catSelect->currentCategory() ) );
    
    constructorDone = TRUE;
    updateIcons();
    
    connect(qApp, SIGNAL(categoriesChanged()), this, SLOT(catChanged()) );

    // This call is to ensure that all records contains only valid categories 
    catChanged();
}

void AddressbookWindow::setDocument( const QString &filename )
{
    receiveFile(filename);
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

    QValueList<PimContact> cl = PimContact::readVCard( targetFile );

    if ( cl.count() == 0 ) {
	QMessageBox::warning(this, tr("Could not read VCard document"),
	    tr("The VCard document did not \ncontain any valid VCards") );
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
		list += (*it).fullName() + "\n";
	    } else if ( count == 3 ) {
		list += "...\n";
	    }
	    count++;
	}

	QString msg = tr("%1 new VCard(s) for \n%2\nDo you want to add them to\nyour addressbook?").arg( newContacts.count() ).arg(list);
	if ( QMessageBox::information(this, tr("New contact(s)"), msg, QMessageBox::Ok, QMessageBox::Cancel) ==
	    QMessageBox::Ok ) {

	    for ( QValueList<PimContact>::Iterator it = newContacts.begin(); it != newContacts.end(); ++it) {
		contacts.addContact( *it );
	    }
	    abList->reload( contacts.sortedContacts() );
	}
    } else if ( oldContacts.count() > 0 ) {
	QString list = "";
	uint count = 0;
	for ( QValueList<PimContact>::Iterator it = oldContacts.begin(); it != oldContacts.end(); ++it) {
	    if ( count < 3 ) {
		list += (*it).fullName() + "\n";
	    } else if ( count == 3 ) {
		list += "...\n";
	    }
	    count++;
	}

	QString msg = tr("%1 old VCard(s) for \n%2\nThe document only contained \nVCards already in your addressbook").arg( oldContacts.count() ).arg(list);
	QMessageBox::information(this, tr("Contact(s) already registered"), msg, QMessageBox::Ok);

    }
}

void AddressbookWindow::flush()
{
    syncing = TRUE;
    contacts.saveData();
}

void AddressbookWindow::reload()
{
    syncing = FALSE;
    QArray<int> vl( 0 );
    catSelect->setCategories( vl, "Address Book", // No tr()
	tr("Address Book") );
    catSelect->setAllCategories( TRUE );
    contacts.ensureDataCurrent(TRUE);
    abList->reload( contacts.sortedContacts() );
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
	slotListView();
    else if ( centralWidget() == mView )
	showView();
}

AddressbookWindow::~AddressbookWindow()
{
    writeConfig();
}

void
AddressbookWindow::readConfig(void)
{
    Config  config("Contacts");
    config.setGroup("default");

    catSelect->setCurrentCategory(config.readNumEntry("Category", -2));
}

void
AddressbookWindow::writeConfig(void)
{
    Config  config("Contacts");
    config.setGroup("default");

    config.writeEntry("Category", catSelect->currentCategory());
}

void AddressbookWindow::updateIcons()
{
    if ( abList->hasCurrentEntry() ) {
	PimContact ce = abList->currentEntry();

	actionNew->setEnabled(TRUE);
	actionEdit->setEnabled(TRUE);
	actionDuplicate->setEnabled(TRUE);
	actionTrash->setEnabled(TRUE);
	actionFind->setEnabled(TRUE);
	actionMail->setEnabled(!ce.defaultEmail().isEmpty());
	if ( abList->selectedContacts().count() == 1 ) {
	    actionSetPersonal->setEnabled(TRUE);
	} else {
	    actionSetPersonal->setEnabled(FALSE);
	}
	if (actionBeam) {
	    actionBeam->setEnabled(TRUE);
	}
    } else {
	actionNew->setEnabled(TRUE);
	actionEdit->setEnabled(FALSE);
	actionDuplicate->setEnabled(FALSE);
	actionTrash->setEnabled(FALSE);
	// If find is open, need to close it first.
	if (actionFind->isOn())
	    actionFind->setOn(FALSE);
	actionFind->setEnabled(FALSE);
	actionMail->setEnabled(FALSE);
	actionSetPersonal->setEnabled(FALSE);
	if (actionBeam) {
	    actionBeam->setEnabled(FALSE);
	}
    }

    actionPersonal->setEnabled( contacts.hasPersonal() );
}

void AddressbookWindow::setViewOptions(bool enabled)
{
    int	id;
    int	i = 0;

    for (i = 0; (id = viewmenu->idAt(i)) != -1; i++) {
	viewmenu->setItemEnabled(id, enabled);
    }
}

void AddressbookWindow::showView()
{
    listView->hide();
    setCentralWidget( abView() );
    mView->show();
    mView->setFocus();
    updateIcons();
    setCaption( tr("Contact Details") );
    actionList->setEnabled( TRUE );
    setViewOptions(FALSE);
}

void AddressbookWindow::slotListNew()
{
    PimContact cnt;
    if( !syncing ) {
	if ( abEditor )
	    abEditor->setEntry( cnt );
	abView()->init( cnt );
	editEntry( NewEntry );
    } else {
	QMessageBox::warning(this, tr("Contacts"),
			     tr("Can not edit data, currently syncing"));
    }
}

void AddressbookWindow::slotDetailView()
{
    if ( abList->hasCurrentEntry() ) {
	if (actionFind->isOn())
	    actionFind->setOn(FALSE);
	abView()->init( abList->currentEntry() );
	mView->sync();
	showView();
    }
}

void AddressbookWindow::slotListView()
{
    if ( !abList->isVisible() ) {
	abView()->hide();
	setCentralWidget( listView );
	listView->show();
	abList->setFocus();
	setCaption( tr("Contacts") );
	actionList->setEnabled( FALSE );
	setViewOptions(TRUE);
    }
    
}

void AddressbookWindow::slotListDelete()
{
    if ( syncing ) {
	QMessageBox::warning( this, tr("Contacts"),
			      tr("Can not edit data, currently syncing") );
	return;
    }
    
    if ( abList->selectionMode() == AbTable::Extended ) {
	QValueList<QUuid> t = abList->selectedContacts();
	
	if ( !t.count() ) return;
	
	QString str;
	if ( t.count() > 1 ) {
	    str = QString("<qt>Are you sure you want to delete the %1 selected contacts?</qt>").arg( t.count() );
	} else {
	    // some duplicated code, but selected/current logic might change, so I'll leave it as it is
	    PimContact tmpEntry = abList->currentEntry();
	    QString strName = tmpEntry.fullName();
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
    
    } else {
	PimContact tmpEntry = abList->currentEntry();

	// get a name, do the best we can...
	QString strName = tmpEntry.fullName();
	if ( strName.isEmpty() ) {
	    strName = tmpEntry.company();
	    if ( strName.isEmpty() )
		strName = tr("No Name");
	}


	if ( QPEMessageBox::confirmDelete( this, tr( "Contacts" ),
					   strName ) ) {
	    contacts.removeContact( abList->currentEntry() );
	    abList->reload( contacts.sortedContacts() );
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
    
    abList->reload( contacts.sortedContacts() );
}


void AddressbookWindow::slotViewBack()
{
    slotListView();
}

void AddressbookWindow::slotViewEdit()
{
    if(!syncing) {
	if ( !bAbEditFirstTime )
	    abEditor->setEntry( abList->currentEntry() );
	editEntry( EditEntry );
    } else {
	QMessageBox::warning( this, tr("Contacts"),
			      tr("Can not edit data, currently syncing") );
    }
}

void AddressbookWindow::slotViewDuplicate(void)
{
    if (!syncing) {
	duplicateEntry();
    } else {
	QMessageBox::warning( this, tr("Contacts"),
			      tr("Can not edit data, currently syncing") );
    }
}


void AddressbookWindow::writeMail()
{
    PimContact c = abList->currentEntry();
    QString name = c.fileAs();
    QString email = c.defaultEmail();
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Email"), "writeMail(QString,QString)");
    e << name << email;
#endif
}




static const char * beamfile = "/tmp/obex/contact.vcf";

void AddressbookWindow::slotBeam()
{
    QString filename, description;
    unlink( beamfile ); // delete if exists
    if ( !abList->hasCurrentEntry() ) {
	qWarning("ab::slotBeam called with nothing to beam");
	return;
    }
#ifndef Q_OS_WIN32
    mkdir("/tmp/obex/", 0755);
#else
    QDir d;
    d.mkdir("/tmp/obex");
#endif
    filename = beamfile;

    if ( abList->selectionMode() == AbTable::Extended && abList->selectedContacts().count() > 0 ) {
	PimContact::writeVCard( beamfile, abList->selected() );
	uint count = abList->selectedContacts().count();
	if ( count > 1 )
	    description = QString("the %1 selected contacts").arg( abList->selectedContacts().count() );
	else
	    description = abList->currentEntry().fullName();

    } else {
	PimContact c = abList->currentEntry();
	PimContact::writeVCard( beamfile, c );
	description = c.fullName();
    }
    Ir *ir = new Ir( this );
    connect( ir, SIGNAL( done( Ir * ) ), this, SLOT( beamDone( Ir * ) ) );
    ir->send( filename, description, "text/x-vCard" );
}

void AddressbookWindow::beamDone( Ir *ir )
{
    delete ir;
    unlink( beamfile );
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

void AddressbookWindow::appMessage(const QCString &msg, const QByteArray &data)
{
    if (msg == "editPersonal()") {
	editPersonal();
    } else if (msg == "editPersonalAndClose()") {
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
	contacts.addContact( c );
	abList->reload( contacts.sortedContacts() );

    } else if ( msg == "removeContact(PimContact)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimContact c;
	stream >> c;
	contacts.removeContact( c );
	abList->reload( contacts.sortedContacts() );
    } else if ( msg == "updateContact(PimContact)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimContact c;
	stream >> c;
	contacts.updateContact( c );
	abList->reload( contacts.sortedContacts() );
    } else if ( msg == "addAndEditContact(PimContact)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimContact c;
	stream >> c;

	if ( bAbEditFirstTime ) {
	    abEditor = new AbEditor( this );
	    abEditor->setEntry( c );
	    bAbEditFirstTime = FALSE;
	} else {
	    abEditor->setEntry( c );
	}
	abView()->init( c );
	editEntry( NewEntry );
    } else if ( msg == "showContact(QUuid)" ) {
	QUuid u;
	QDataStream stream(data, IO_ReadOnly);
	stream >> u;
	abList->setCurrentEntry(u);
	slotListView();
#ifdef Q_WS_QWS
    } else if ( msg == "beamBusinessCard()" ) {


	QString filename, description;
	unlink( beamfile ); // delete if exists
#ifndef Q_OS_WIN32
	mkdir("/tmp/obex/", 0755);
#else
	QDir d;
	d.mkdir("/tmp/obex");
#endif
	filename = beamfile;

	PimContact c((const PimContact &)contacts.personal());
	PimContact::writeVCard( beamfile, c );
	description = c.fullName();
	Ir *ir = new Ir( this );
	connect( ir, SIGNAL( done( Ir * ) ), this, SLOT( beamDone( Ir * ) ) );
	ir->send( filename, description, "text/x-vCard" );
#endif
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
    
    abView()->init( me );
    abView()->sync();
    listView->hide();
    setCentralWidget( abView() );
    mView->show();
    mView->setFocus();
    actionList->setEnabled( TRUE );
}

// hacky static bool
static bool newPersonal = FALSE;

void AddressbookWindow::editPersonal()
{
    if (contacts.hasPersonal()) {
	PrContact me(contacts.personal());
	if ( bAbEditFirstTime ) {
	    abEditor = new AbEditor( this );
	    bAbEditFirstTime = FALSE;
	}
	abEditor->setEntry( me );
	abView()->init( me );
	newPersonal = FALSE;
	editEntry( EditEntry );
    } else {
	newPersonal = TRUE;
	editEntry( NewEntry );
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
	contacts.setAsPersonal(c.uid());
	abList->reload( contacts.sortedContacts() );
	updateIcons();
    }
}

void AddressbookWindow::viewNext()
{
    if ( abList->hasCurrentEntry() ) {
	int cr = abList->currentRow();
	if ( ++cr < abList->numRows() ) {
	    abList->setCurrentCell( cr, 0 );
	    abView()->init( abList->currentEntry() );
	    mView->sync();
	}
    }
}

void AddressbookWindow::viewPrevious()
{
    if ( abList->hasCurrentEntry() ) {
	int cr = abList->currentRow();
	if ( --cr >= 0 ) {
	    abList->setCurrentCell( cr, 0 );
	    abView()->init( abList->currentEntry() );
	    mView->sync();
	}
    }
}

void AddressbookWindow::editEntry( EntryMode entryMode )
{
    PimContact entry;
    if ( bAbEditFirstTime ) {
	abEditor = new AbEditor( this );
	abEditor->setEntry( entry );
	bAbEditFirstTime = FALSE;
	if ( entryMode == EditEntry )
	    abEditor->setEntry( abList->currentEntry() );
    }
    // fix the foxus...
    abEditor->setNameFocus();

    if ( QPEApplication::execDialog(abEditor) ) {
	setFocus();
	if ( entryMode == NewEntry ) {
	    PimContact insertEntry = abEditor->entry();
	    QUuid ui = contacts.addContact( insertEntry );
	    if (newPersonal)
		contacts.setAsPersonal(ui);
	    abList->reload( contacts.sortedContacts() );
	    updateIcons();
	} else {
	    PimContact replaceEntry = abEditor->entry();
	    contacts.updateContact( replaceEntry );
	    abList->reload( contacts.sortedContacts() );
	}
    }
    if ( centralWidget() == abView() )
	slotDetailView();
}

void AddressbookWindow::duplicateEntry(void)
{
    PimContact duplicate;

    if (abList->hasCurrentEntry()) {
	duplicate = abList->currentEntry();
	duplicate.setLastName(tr("%1 (duplicate)","Noun").
	    arg(duplicate.lastName()));

	duplicate.setFileAs();

	contacts.addContact( duplicate );
	abList->reload( contacts.sortedContacts() );
	slotListView();
    }
}

void AddressbookWindow::closeEvent( QCloseEvent *e )
{
    // in case we are showing details, close that view
    slotListView();
    e->accept();
}


AbLabel *AddressbookWindow::abView()
{
    if ( !mView ) {
      mView = new AbLabel( this );
      mView->init( PimContact()  );
      connect( mView, SIGNAL(okPressed()), this, SLOT( slotListView() ) );
      connect( mView, SIGNAL(previous()), this, SLOT( viewPrevious() ) );
      connect( mView, SIGNAL(next()), this, SLOT( viewNext() ) );
    }
    return mView;
}

void AddressbookWindow::slotFind(bool s)
{
    if ( s ) {
	if ( centralWidget() == abView() )
	    slotListView();
	searchBar->show();
	searchEdit->setFocus();
    } else {
	searchBar->hide();
	if ( abList->numSelections() )
	    abList->clearSelection();
	abList->clearFindRow();
    }
}

void AddressbookWindow::search()
{
    abList->findNext( searchEdit->text(), contacts.filter() );
}

void AddressbookWindow::findWrapped()
{
    Global::statusMessage( tr("Find: reached end") );
}

void AddressbookWindow::findNotFound()
{
    Global::statusMessage( tr("Find: not found") );
}

void AddressbookWindow::findFound()
{
    Global::statusMessage( "" );
}

void AddressbookWindow::slotSetCategory( int c )
{
    //abList->setPaintingEnabled( false );
    contacts.setFilter( c );
    abList->reload( contacts.sortedContacts() );
    //abList->setPaintingEnabled( true );
    setCaption( tr("Contacts") + " - " + categoryLabel( c ) );
}

// Loop through and validate the categories.  If any records' category list was
// modified we need to update
void AddressbookWindow::catChanged()
{
    QListIterator<PrContact> it(contacts.contacts() );
    Categories c;
    c.load( categoryFileName() );
    QArray<int> cats = c.ids("Address Book", c.labels("Address Book", TRUE) ); // No tr
    bool changed = FALSE;
    for(; it.current(); ++it) {
	PimContact t( *(*it) );
	if ( t.pruneDeadCategories( cats ) ) {
    	    contacts.updateContact( t );
	    changed = TRUE;
	}
    }
    
    if ( changed )
	abList->reload( contacts.sortedContacts() );
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
    return cats.label( "Address Book", // No tr()
	id );
}

void AddressbookWindow::contactFilterSelected( int idx )
{
    int cid = contactMap[idx];
    abList->setPreferredField( cid );
    abList->reload( contacts.sortedContacts() );
}

void AddressbookWindow::selectAll()
{
    abList->selectAll();
}

void AddressbookWindow::configure()
{
    AbSettings settings(this, "", TRUE);
    settings.setCurrentFields( abList->fields() );
    if ( QPEApplication::execDialog(&settings) == QDialog::Accepted ) {
	abList->setFields( settings.fields() );
	abList->reload( contacts.sortedContacts() );
    }
}

