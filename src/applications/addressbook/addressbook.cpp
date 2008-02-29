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
#include "addresssettings.h"
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
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/services/services.h>
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
#include <unistd.h>

#include <qdatetime.h>

static QString addressbookPersonalVCardName()
{
    QString filename = Global::applicationFileName("addressbook",
	    "businesscard.vcf");
    return filename;
}


AddressbookWindow::AddressbookWindow( QWidget *parent, const char *name,
				      WFlags f )
    : QMainWindow( parent, name, f ),
      abEditor(0),
      bAbEditFirstTime(TRUE),
      syncing(FALSE)
{
    initFields();

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

    listTools = new QPEToolBar( this, "list operations" );


    QAction *a = new QAction( tr( "New" ), Resource::loadIconSet( "new" ), QString::null,
			      0, this, 0 );
    actionNew = a;
    connect( a, SIGNAL( activated() ), this, SLOT( slotListNew() ) );
    a->setWhatsThis( tr("Enter a new contact.") );
    a->addTo( edit );
    a->addTo( listTools );

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
    a->addTo( listTools );

    a = new QAction( tr( "Delete" ), Resource::loadIconSet( "trash" ), QString::null,
		     0, this, 0 );
    actionTrash = a;
    a->setWhatsThis( tr("Delete the highlighted contact.") );
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
	a->setWhatsThis( tr("Beam this contact to another device.") );
	a->addTo( edit );
	a->addTo( listTools );
    }else
      actionBeam = 0L;

    edit->insertSeparator();

    a = new QAction( tr("My Personal Details"), QString::null, 0, 0, 0, TRUE );
    actionPersonal = a;
    a->setWhatsThis( tr("Edit your personal contact details.") );
    connect( a, SIGNAL( activated() ), this, SLOT( slotPersonalView() ) );
    a->addTo( edit );

    a = new QAction( tr( "Arrange Edit Fields"), QString::null, 0, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( slotSettings() ) );
    a->setWhatsThis( tr("Select the order that the fields will appear when editing contacts.") );
    a->addTo( edit );

    // Search bar
    searchBar = new QPEToolBar(this);
    addToolBar( searchBar,  "Search", QMainWindow::Top, TRUE );

    searchBar->setHorizontalStretchable( TRUE );

    searchEdit = new QLineEdit( searchBar, "searchEdit" );
    searchBar->setStretchableWidget( searchEdit );
    connect( searchEdit, SIGNAL(textChanged(const QString &)),
	    this, SLOT(search()) );
    connect( searchEdit, SIGNAL(returnPressed()), this, SLOT(search()) );

    a = new QAction( tr( "Find Next" ), Resource::loadPixmap( "next" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL(activated()), this, SLOT(search()) );
    a->setWhatsThis( tr("Find the next occurrence of the search text.") );
    a->addTo( searchBar );

    searchBar->hide();

    // Create Views

    listView = new QVBox( this );

    abList = new AbTable( &orderedFields, listView, "table" );
    abList->setFrameStyle( QFrame::NoFrame );
    abList->setHScrollBarMode( QScrollView::AlwaysOff );
    connect( abList, SIGNAL( empty( bool ) ),
	     this, SLOT( listIsEmpty( bool ) ) );
    connect( abList, SIGNAL( details() ),
	     this, SLOT( slotListView() ) );
    connect( abList, SIGNAL(currentChanged(int,int)),
	     this, SLOT(slotUpdateToolbar(int, int)) );
    connect( abList, SIGNAL(findWrapAround()), this, SLOT(findWrapped()) );
    connect( abList, SIGNAL(findNotFound()), this, SLOT(findNotFound()) );
    connect( abList, SIGNAL(findFound()), this, SLOT(findFound()) );
    QWhatsThis::add( abList, tr("List of contacts in the selected category.  Click to view detailed information.") );

    mView = 0;

    // Filter bar
    QHBox *hb = new QHBox( listView );

    QComboBox *contactCombo = new QComboBox( hb );
    connect( contactCombo, SIGNAL(activated(int)), this, SLOT(contactFilterSelected(int)) );
    contactCombo->insertItem( tr("Email") );
    contactMap[0]=Qtopia::Emails;
    contactCombo->insertItem( tr("Business Phone") );
    contactMap[1]=Qtopia::BusinessPhone;
    contactCombo->insertItem( tr("Home Phone") );
    contactMap[2]=Qtopia::HomePhone;
    contactCombo->setCurrentItem(0);
    contactFilterSelected( 0 );
    QWhatsThis::add( contactCombo, tr("Show this contact information in the Contact column, if available.") );

    Categories c;
    c.load(categoryFileName());
    QArray<int> vl( 0 );
    catSelect = new CategorySelect( hb );
    catSelect->setRemoveCategoryEdit( TRUE );
    catSelect->setCategories( vl, "Address Book", tr("Address Book") );
    catSelect->setAllCategories( TRUE );
    connect( catSelect, SIGNAL(signalSelected(int)), this, SLOT(slotSetCategory(int)) );
    QWhatsThis::add( catSelect, tr("Show contacts in this category.") );

    setCentralWidget( listView );
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
	    tr("The VCard document did not \ncontain any valid VCards"), tr("Ok") );
	return;
    }
    
    QValueList<PimContact> newContacts, oldContacts;
    for( QValueList<PimContact>::Iterator it = cl.begin(); it != cl.end(); ++it ) {
	    if ( abList->hasEntry(*it) ) {
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
		abList->addEntry( *it );
	    }
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
    abList->flush();
}

void AddressbookWindow::reload()
{
    syncing = FALSE;
    QArray<int> vl( 0 );
    catSelect->setCategories( vl, "Address Book", tr("Address Book") );
    catSelect->setAllCategories( TRUE );
    abList->reload();
}

void AddressbookWindow::resizeEvent( QResizeEvent *e )
{
  QMainWindow::resizeEvent( e );

  if ( centralWidget() == listView  )
      showList();
  else if ( centralWidget() == mView )
      showView();
}

AddressbookWindow::~AddressbookWindow()
{
}

void AddressbookWindow::slotUpdateToolbar(int row, int col)
{
    //
    // (-1, -1) indicates editing personal details.
    //
    if (row == -1 && col == -1) {
	actionFind->setEnabled(FALSE);
	actionNew->setEnabled(FALSE);
	actionEdit->setEnabled(TRUE);
	actionDuplicate->setEnabled(FALSE);
	actionTrash->setEnabled(FALSE);
	actionFind->setEnabled(FALSE);
	actionMail->setEnabled(FALSE);
	if (actionBeam)
	  actionBeam->setEnabled(TRUE);
    } else {
	if (abList->numRows() > 0) {
	    PimContact ce = abList->currentEntry();

	    actionNew->setEnabled(TRUE);
	    actionEdit->setEnabled(TRUE);
	    actionDuplicate->setEnabled(TRUE);
	    actionTrash->setEnabled(TRUE);
	    actionFind->setEnabled(TRUE);
	    actionMail->setEnabled(!ce.defaultEmail().isEmpty());
	    if (actionBeam)
	      actionBeam->setEnabled(TRUE);
	} else {
	    actionNew->setEnabled(TRUE);
	    actionEdit->setEnabled(FALSE);
	    actionDuplicate->setEnabled(FALSE);
	    actionTrash->setEnabled(FALSE);
	    actionFind->setEnabled(FALSE);
	    actionMail->setEnabled(FALSE);
	    if (actionBeam)
	      actionBeam->setEnabled(FALSE);
	}
    }
}

void AddressbookWindow::showList()
{
    if ( mView ) mView->hide();
    setCentralWidget( listView );
    listView->show();
    // update our focues... (or use a stack widget!);
    abList->setFocus();
    slotUpdateToolbar(0, 0);
    setCaption( tr("Contacts") );
}

void AddressbookWindow::showView()
{
    if ( abList->numRows() > 0 ) {
	listView->hide();
	setCentralWidget( abView() );
	mView->show();
	mView->setFocus();
	slotUpdateToolbar(0, 0);
	setCaption( tr("Contact Details") );
    }
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

void AddressbookWindow::slotListView()
{
    abView()->init( abList->currentEntry() );
    mView->sync();
    showView();
}

void AddressbookWindow::slotListDelete()
{
    if(!syncing) {
	PimContact tmpEntry = abList->currentEntry();

	// get a name, do the best we can...
	QString strName = tmpEntry.fullName();
	if ( strName.isEmpty() ) {
	    strName = tmpEntry.company();
	    if ( strName.isEmpty() )
		strName = "No Name";
	}


	if ( QPEMessageBox::confirmDelete( this, tr( "Contacts" ),
					   strName ) ) {
		abList->deleteCurrentEntry();
		showList();
	}
    } else {
	QMessageBox::warning( this, tr("Contacts"),
			      tr("Can not edit data, currently syncing") );
    }
}

void AddressbookWindow::slotViewBack()
{
    showList();
}

void AddressbookWindow::slotViewEdit()
{
    if(!syncing) {
	if (actionPersonal->isOn()) {
	    editPersonal();
	} else {
	    if ( !bAbEditFirstTime )
		abEditor->setEntry( abList->currentEntry() );
	    editEntry( EditEntry );
	}
    } else {
	QMessageBox::warning( this, tr("Contacts"),
			      tr("Can not edit data, currently syncing") );
    }
}

void AddressbookWindow::slotViewDuplicate(void)
{
    if (!syncing) {
	if (! actionPersonal->isOn()) {
	    duplicateEntry();
	}
    } else {
	QMessageBox::warning( this, tr("Contacts"),
			      tr("Can not edit data, currently syncing") );
    }
}


void AddressbookWindow::writeMail()
{
    PimContact c = abList->currentEntry();
    QString name = c.fullName();
    QString email = c.defaultEmail();
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Email"), "writeMail(QString,QString)");
    e << name << email;
#endif
}




static const char * beamfile = "/tmp/obex/contact.vcf";

void AddressbookWindow::slotBeam()
{
    QString filename;
    PimContact c;
    if ( actionPersonal->isOn() ) {
	filename = addressbookPersonalVCardName();
	if (!QFile::exists(filename))
	    return; // can't beam a non-existent file
	c = PimContact::readVCard( filename )[0];
    } else {
	unlink( beamfile ); // delete if exists
	c = abList->currentEntry();
	mkdir("/tmp/obex/", 0755);
	PimContact::writeVCard( beamfile, c );
	filename = beamfile;
    }
    Ir *ir = new Ir( this );
    connect( ir, SIGNAL( done( Ir * ) ), this, SLOT( beamDone( Ir * ) ) );
    QString description = c.fullName();
    ir->send( filename, description, "text/x-vCard" );
}

void AddressbookWindow::beamDone( Ir *ir )
{
    delete ir;
    unlink( beamfile );
}


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
	abList->addEntry(c);
    } else if ( msg == "removeContact(PimContact)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimContact c;
	stream >> c;
	abList->deleteEntry(c);
    } else if ( msg == "updateContact(PimContact)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimContact c;
	stream >> c;
	abList->updateEntry(c);
    } else if ( msg == "addAndEditContact(PimContact)" ) {
	QDataStream stream(data,IO_ReadOnly);
	PimContact c;
	stream >> c;
	
	if ( bAbEditFirstTime ) {
	    abEditor = new AbEditor( c, &orderedFields, &slOrderedFields,
				     this, "editor" );
	    bAbEditFirstTime = FALSE;
	} else {
	    abEditor->setEntry( c );	
	}
	abView()->init( c );
	editEntry( NewEntry );
    } else if ( msg == "beamBusinessCard()" ) {
	QUuid u;
	QDataStream stream(data, IO_ReadOnly);
	stream >> u;
	abList->setCurrentEntry(u);
	slotListView();
    } else if ( msg == "beamBusinessCard()" ) {
      QString card = getenv("HOME");
      card += "/Applications/addressbook/businesscard.vcf";
      if ( QFile::exists( card ) ) {
	QCopEnvelope e("QPE/Obex", "send(QString,QString,QString)");
	QString mimetype = "text/x-vCard";
	e << tr("business card") << card << mimetype;
      }
    }
}

void AddressbookWindow::editPersonal()
{
    QString filename = addressbookPersonalVCardName();
    PimContact me;
    if (QFile::exists(filename))
	me = PimContact::readVCard( filename )[0];
    if (bAbEditFirstTime) {
	abEditor = new AbEditor( me, &orderedFields, &slOrderedFields,
		this, "editor" );
	// don't create a new editor every time
	bAbEditFirstTime = FALSE;
    } else
	abEditor->setEntry( me );

    abEditor->setCaption(tr("Edit My Personal Details"));
    // fix the foxus...
    abEditor->setNameFocus();

    if ( QPEApplication::execDialog(abEditor) ) {
	setFocus();
	PimContact new_personal = abEditor->entry();
	QString fname = addressbookPersonalVCardName();
	PimContact::writeVCard( fname, new_personal );
	abView()->init(new_personal);
	abView()->sync();
    }
    abEditor->setCaption( tr("Edit Address") );
}

void AddressbookWindow::slotPersonalView()
{
    if (!actionPersonal->isOn()) {
	// we just turned it off
	setCaption( tr("Contacts") );
	slotUpdateToolbar(0, 0);
	showList();
	return;
    }

    slotUpdateToolbar(-1, -1);

    setCaption( tr("Contacts - My Personal Details") );
    QString filename = addressbookPersonalVCardName();
    PimContact me;
    if (QFile::exists(filename))
	me = PimContact::readVCard( filename )[0];

    abView()->init( me );
    abView()->sync();
    listView->hide();
    setCentralWidget( abView() );
    mView->show();
    mView->setFocus();
}

void AddressbookWindow::editEntry( EntryMode entryMode )
{
    PimContact entry;
    if ( bAbEditFirstTime ) {
	abEditor = new AbEditor( entry, &orderedFields, &slOrderedFields,
				 this, "editor" );
	bAbEditFirstTime = FALSE;
	if ( entryMode == EditEntry )
	    abEditor->setEntry( abList->currentEntry() );
    }
    // other things may chane the caption.
    abEditor->setCaption( tr("Edit Address") );
    // fix the foxus...
    abEditor->setNameFocus();

    if ( QPEApplication::execDialog(abEditor) ) {
	setFocus();
	if ( entryMode == NewEntry ) {
	    PimContact insertEntry = abEditor->entry();
	    abList->addEntry( insertEntry );
	} else {
	    PimContact replaceEntry = abEditor->entry();
	    abList->updateEntry( replaceEntry );
	}
    }
    showList();
}

void AddressbookWindow::duplicateEntry(void)
{
    PimContact existing = abList->currentEntry();
    PimContact duplicate;

    duplicate = abList->currentEntry();
    duplicate.setLastName(duplicate.lastName() + " (duplicate)");
    duplicate.setFileAs();

    abList->addEntry(duplicate);
    showList();
}

void AddressbookWindow::listIsEmpty( bool )
{
    slotUpdateToolbar(0, 0);
}

void AddressbookWindow::closeEvent( QCloseEvent *e )
{
    if ( centralWidget() == mView ) {
	if (actionPersonal->isOn()) {
	    // pretend we clicked it off
	    actionPersonal->setOn(FALSE);
	    slotPersonalView();
	} else {
	    showList();
	}
	e->ignore();
	return;
    }
    e->accept();
}

void AddressbookWindow::slotSettings()
{
    AddressSettings frmSettings( this );

    if ( QPEApplication::execDialog(&frmSettings) ) {
	allFields.clear();
	orderedFields.clear();
	slOrderedFields.clear();
	initFields();
 	if ( abEditor )
 	    abEditor->loadFields();
 	abList->refresh();
    }
}


void AddressbookWindow::initFields()
{
    // we really don't need the things from the configuration, anymore
    // only thing that is important are the important categories.  So,
    // Call the contact functions that correspond to these old functions...

    QStringList xmlFields = PimContact::fields();
    QStringList visibleFields = PimContact::trfields();
    xmlFields.remove( "Title" );
    visibleFields.remove( tr("Name Title") );
    visibleFields.remove( tr("Notes") );

    xmlFields.remove( "Pronunciation" );
    xmlFields.remove( "Rid" );
    xmlFields.remove( "RidInfo" );
    xmlFields.remove( "Groups" );
    visibleFields.remove( "Pronunciation" );
    visibleFields.remove( "Rid" );
    visibleFields.remove( "RidInfo" );
    visibleFields.remove( "Groups" );

    int i,
	version;
    Config cfg( "AddressBook" );
    QString zn;

    // ### Write a function to keep this from happening again...
    QStringList::ConstIterator it;
    for ( i = 0, it = xmlFields.begin(); it != xmlFields.end(); ++it, i++ ) {
	allFields.append( i + 3 );
    }

    cfg.setGroup( "Version" );
    version = cfg.readNumEntry( "version" );
    i = 0;
    if ( version >= ADDRESSVERSION ) {

	cfg.setGroup( "ImportantCategory" );

	zn = cfg.readEntry( "Category" + QString::number(i), QString::null );
	while ( !zn.isNull() ) {
	    if ( zn.contains( tr("Work") ) || zn.contains( tr("Mb") ) ) {
		slOrderedFields.clear();
		break;
	    }
	    slOrderedFields.append( zn );
	    zn = cfg.readEntry( "Category" + QString::number(++i), QString::null );
	}
    } else {
	QString str;
	str = getenv("HOME");
	str += "/Settings/AddressBook.conf";
	QFile::remove( str );
    }
    if ( slOrderedFields.count() > 0 ) {
	for( QStringList::ConstIterator it = slOrderedFields.begin();
	     it != slOrderedFields.end(); ++it ) {
	    QValueList<int>::ConstIterator itVl;
	    QStringList::ConstIterator itVis;
	    itVl = allFields.begin();
	    for ( itVis = visibleFields.begin();
		  itVis != visibleFields.end() && itVl != allFields.end();
		  ++itVis, ++itVl ) {
		if ( *it == *itVis && itVl != allFields.end() ) {
		    orderedFields.append( *itVl );
		}
	    }
	}
    } else {
	QValueList<int>::ConstIterator it;
	for ( it = allFields.begin(); it != allFields.end(); ++it )
	    orderedFields.append( *it );

	slOrderedFields = visibleFields;
	orderedFields.remove( Qtopia::AddressUid );
	orderedFields.remove( Qtopia::Title );
	orderedFields.remove( Qtopia::Groups );
	orderedFields.remove( Qtopia::AddressCategory );
	orderedFields.remove( Qtopia::FirstName );
	orderedFields.remove( Qtopia::LastName );
	orderedFields.remove( Qtopia::DefaultEmail );
	orderedFields.remove( Qtopia::FileAs );
	orderedFields.remove( Qtopia::Notes );
	orderedFields.remove( Qtopia::Gender );
	slOrderedFields.remove( tr("Name Title") );
        slOrderedFields.remove( tr("First Name") );
        slOrderedFields.remove( tr("Last Name") );
        slOrderedFields.remove( tr("File As") );
        slOrderedFields.remove( tr("Default Email") );
        slOrderedFields.remove( tr("Notes") );
        slOrderedFields.remove( tr("Gender") );

    }
}


AbLabel *AddressbookWindow::abView()
{
    if ( !mView ) {
      mView = new AbLabel( this, "viewer" );
      mView->init( PimContact()  );
      connect( mView, SIGNAL( okPressed() ), this, SLOT( slotListView() ) );
    }
    return mView;
}

void AddressbookWindow::slotFind(bool s)
{
    if ( s ) {
	if ( centralWidget() == abView() )
	    showList();
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
    abList->slotDoFind( searchEdit->text(), abList->showCategory() );
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
    abList->setShowCategory( c );
    abList->refresh();
    //abList->setPaintingEnabled( true );
    setCaption( tr("Contacts") + " - " + abList->categoryLabel( c ) );
}

void AddressbookWindow::contactFilterSelected( int idx )
{
    int cid = contactMap[idx];
    abList->setPreferredField( cid );
    abList->refresh();
}
