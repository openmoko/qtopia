/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/**********************************************************************
** This file is part of the KPhone project.
** 
** KPhone is a SIP (Session Initiation Protocol) user agent for Linux, with
** which you can initiate VoIP (Voice over IP) connections over the Internet,
** send Instant Messages and subscribe your friends' presence information.
** Read INSTALL for installation instructions, and CHANGES for the latest
** additions in functionality and COPYING for the General Public License
** (GPL).
** 
** More information about Wirlab available at http://www.wirlab.net/
** 
** Note: "This program is released under the GPL with the additional
** exemption that compiling, linking, and/or using OpenSSL is allowed."
***********************************************************************/
#include <stdio.h>
#include <qstring.h>
#include <qregexp.h>
#include <qlistview.h>
#include <qfile.h>
#include <qxml.h>
#include <qwindowdefs.h>
//PD #include <qmessagebox.h>
#include <qsplitter.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qdom.h>
#include <qurloperator.h>
#include <menulistview.h>

#include "kphonebook.h"


IncomingCall::IncomingCall( QString s, QDateTime time )
{
	contact = s;
	dt = time;
}

IncomingCall::~IncomingCall( void )
{
}

PhoneBookAddIdentity::PhoneBookAddIdentity( QWidget *parent, const char *name )
	: QDialog( parent, name, true )
{
	setCaption( QString( name ) );
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );

	(void) new QLabel( tr("Person:"), vbox );
	person = new QLineEdit( vbox );
	(void) new QLabel( tr("Identity:"), vbox );
	identity = new QLineEdit( vbox );
	(void) new QLabel( tr("Description:"), vbox );
	description = new QLineEdit( vbox );
	contact = new QCheckBox( tr("Add Identity to Contacts List"), vbox );
	description->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	QSpacerItem *spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonBox->addItem( spacer );
	okPushButton = new QPushButton( this, tr("ok button") );
	okPushButton->setText( tr("OK") );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this, tr("cancel button") );
	cancelPushButton->setText( tr("Cancel") );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ), this, SLOT( slotOk() ) );
	connect( cancelPushButton, SIGNAL( clicked() ), this, SLOT( slotCancel() ) );
}

void PhoneBookAddIdentity::clean( void )
{
	person->setText("");
	identity->setText("");
	description->setText("");
	contact->setChecked( false );
}

void PhoneBookAddIdentity::setPerson( QString newPerson )
{
	person->setText( newPerson );
}

void PhoneBookAddIdentity::setIdentity( QString newIdentity )
{
	identity->setText( newIdentity );
}

void PhoneBookAddIdentity::setDescription( QString newDescription )
{
	description->setText( newDescription );
}

void PhoneBookAddIdentity::setContact( bool yesno )
{
	contact->setChecked( yesno );
}

QString PhoneBookAddIdentity::getPerson( void )
{
	return person->text();
}

QString PhoneBookAddIdentity::getIdentity( void )
{
	return identity->text();
}

QString PhoneBookAddIdentity::getDescription( void )
{
	return description->text();
}

QString PhoneBookAddIdentity::getContact( void )
{
	if( contact->isChecked() ){
		return "Yes";
	} else {
		return "No";
	}
}

void PhoneBookAddIdentity::slotOk( void )
{
	QDialog::accept();
}

void PhoneBookAddIdentity::slotCancel( void )
{
	QDialog::reject();
}

PhoneBookEditIdentity::PhoneBookEditIdentity( QWidget *parent, const char *name )
		: QDialog( parent, name, true )
{
	setCaption( QString( name ) );
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );

	(void) new QLabel( tr("Identity:"), vbox );
	identity = new QLineEdit( vbox );
	identity->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	(void) new QLabel( tr("Description:"), vbox );
	description = new QLineEdit( vbox );
	contact = new QCheckBox( tr("Add Identity to Contacts List"), vbox );

// OK, Cancel
	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	QSpacerItem *spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonBox->addItem( spacer );
	okPushButton = new QPushButton( this, "ok button" );
	okPushButton->setText( tr("OK") );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this, "cancel button" );
	cancelPushButton->setText( tr("Cancel") );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ), this, SLOT( slotOk() ) );
	connect( cancelPushButton, SIGNAL( clicked() ), this, SLOT( slotCancel() ) );
}

PhoneBookEditIdentity::~PhoneBookEditIdentity( void )
{
}

QString PhoneBookEditIdentity::getIdentity( void )
{
	return identity->text();
}

QString PhoneBookEditIdentity::getDescription( void )
{
	return description->text();
}

void PhoneBookEditIdentity::setIdentity( QString newIdentity )
{
	identity->setText( newIdentity );
}

void PhoneBookEditIdentity::setDescription( QString newDescription )
{
	description->setText( newDescription );
}

QString PhoneBookEditIdentity::getContact( void )
{
	if( contact->isChecked() ){
		return "Yes";
	} else {
		return "No";
	}
}

void PhoneBookEditIdentity::setContact( QString newContact )
{
	if( newContact.lower() == "yes" ) {
		contact->setChecked( true );
	} else {
		contact->setChecked( false );
	}
}

void PhoneBookEditIdentity::slotOk( void )
{
	QDialog::accept();
}

void PhoneBookEditIdentity::slotCancel( void )
{
	QDialog::reject();
}

PhoneBookEditPerson::PhoneBookEditPerson( QWidget *parent, const char *name )
	: QDialog( parent, name, true )
{
	setCaption( QString( name ) );
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );

	(void) new QLabel( tr("Name:"), vbox );
	person = new QLineEdit( vbox );
	person->setMinimumWidth( fontMetrics().maxWidth() * 20 );

// OK, Cancel
	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	QSpacerItem *spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonBox->addItem( spacer );
	okPushButton = new QPushButton( this, "ok button" );
	okPushButton->setText( tr("OK") );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this, "cancel button" );
	cancelPushButton->setText( tr("Cancel") );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ), this, SLOT( slotOk() ) );
	connect( cancelPushButton, SIGNAL( clicked() ), this, SLOT( slotCancel() ) );
}

PhoneBookEditPerson::~PhoneBookEditPerson( void )
{
}

QString PhoneBookEditPerson::getPerson( void )
{
	return person->text();
}

void PhoneBookEditPerson::setPerson( QString newPerson )
{
	person->setText( newPerson );
}

void PhoneBookEditPerson::slotOk( void )
{
	QDialog::accept();
}

void PhoneBookEditPerson::slotCancel( void )
{
	QDialog::reject();
}

PhoneBookParser::PhoneBookParser( QListView * t, bool CallRegister, bool rejectedContact )
	: QXmlDefaultHandler()
{
	rejected = 0;
	listReceivedCalls = 0;
	listMissedCalls = 0;
	if( rejectedContact ) {
		rejected = new QListViewItem( t, "Permanently rejected contacts", "" );
	}
	if( CallRegister ) {
		QListViewItem *item;
		item = new QListViewItem( t, "Call Register", "" );
		item->setOpen( TRUE );
		listReceivedCalls = new QListViewItem( item, "Received calls", "" );
		listMissedCalls = new QListViewItem( item, "Missed calls", "" );
	}
	listPhoneBook = new QListViewItem( t, "PhoneBook", "" );
	listPhoneBook->setOpen( TRUE );
}

bool PhoneBookParser::startDocument()
{
	return TRUE;
}

bool PhoneBookParser::startElement( const QString&, const QString&,
		const QString& qName,
		const QXmlAttributes& attributes)
{
	QListViewItem *element = 0;
	if( qName == "person" ) {
		element = new QListViewItem( listPhoneBook, attributes.value( "Name" ), "" );
	}
	if( element != 0 ) {
		stack.push( element );
	}
	if( qName == "identity" ) {
		QString uri = attributes.value( "uri" );
		QString description = attributes.value( "description" );
		if( uri.left( 4 ).lower() != "sip:" ) {
			uri = "sip:" + uri;
		}
		QString contact = attributes.value( "contact" );
		if( contact.isEmpty() ) {
			contact = "No";
		}
		if( !stack.isEmpty() ) {
			new QListViewItem( stack.top(), uri, contact, description );
		}
	}
	return TRUE;
}

bool PhoneBookParser::endElement( const QString&, const QString&, const QString &qName )
{
	if ( qName == "person" ){
		stack.pop();
	}
	return TRUE;
}

PhoneBook::PhoneBook(QString fileUserInfo, QWidget *parent, const char *name,
		QList<IncomingCall> &receivedCalls, QList<IncomingCall> &missedCalls,
		QStringList &rejectedContacts )
	: QDialog( parent, name, TRUE )
{
	filePhoneBook = fileUserInfo;
	QFile xmlFile( fileUserInfo );
	QXmlInputSource source( xmlFile );
	QXmlSimpleReader reader;
	resize( 400, 200 );
	caption = QString( name );
	setCaption( QString( caption ) );
	QVBoxLayout *vbox = new QVBoxLayout( this );
	QGridLayout *headergrid = new QGridLayout( 3, 1 );
	vbox->insertLayout( 0, headergrid );
	phoneBook = new MenuListView( this );

	phoneBook->addMenuItem( tr("Add"), this, SLOT( clickAddButton() ) );
	phoneBook->addMenuItem( tr("Edit"), this, SLOT( clickEditButton() ) );
	phoneBook->addMenuItem( tr("Remove"), this, SLOT( clickRemoveButton() ) );
	phoneBook->setMinimumWidth( fontMetrics().maxWidth() * 15 );

	vbox->insertWidget( 1, phoneBook );
	connect( phoneBook, SIGNAL( clicked( QListViewItem * ) ),
		this, SLOT( setIdentity( QListViewItem * ) ) );
	connect( phoneBook, SIGNAL( doubleClicked( QListViewItem * ) ),
		this, SLOT( setIdentityAndDone( QListViewItem * ) ) );
	phoneBook->setSorting( 0 );
	phoneBook->addColumn( tr("Identity") );
	phoneBook->addColumn( tr("Contact") );
	phoneBook->addColumn( tr("Description") );
	if( receivedCalls.count() == 0 && missedCalls.count() == 0 ) {
		if( rejectedContacts.count() == 0 ) {
			parser = new PhoneBookParser( phoneBook, false, false );
		} else {
			parser = new PhoneBookParser( phoneBook, false, true );
		}
	} else {
		parser = new PhoneBookParser( phoneBook );
	}
	reader.setContentHandler( parser );
	reader.parse( source );
	IncomingCall *c;
	QListViewItem *item;
	item = parser->getListReceivedCalls();
	int count = 1;
	QString s;
	for ( c = receivedCalls.last(); c != 0; c = receivedCalls.prev() ) {
		if( count > 99 ) {
			break;
		}
		s.setNum( count++ );
		if( count <= 10 ) {
			s = " " + s;
		}
		new QListViewItem( item, s + ". " + c->getContact(), c->getDateTime().toString() );
	}
	item = parser->getListMissedCalls();
	count = 1;
	for ( c = missedCalls.last(); c != 0; c = missedCalls.prev() ) {
		if( count > 99 ) {
			break;
		}
		s.setNum( count++ );
		if( count <= 10 ) {
			s = " " + s;
		}
		new QListViewItem( item, s + ". " + c->getContact(), c->getDateTime().toString() );
	}

	item = parser->getRejectedContacts();
	for ( QStringList::Iterator it = rejectedContacts.begin(); it != rejectedContacts.end(); ++it ) {
		new QListViewItem( item, QString(*it) );
	}

	QHBox *buttonbox = new QHBox( this );
	buttonbox->setSpacing( 5 );
	vbox->insertWidget( 2, buttonbox );
	QHBox *hb1 = new QHBox( this );
	hb1->setSpacing( 5 );
	vbox->insertWidget( 3, hb1 );
	add_button = new QPushButton( tr("Add"), hb1 );
	connect( add_button, SIGNAL( clicked() ), this, SLOT( clickAddButton() ) );
	edit_button = new QPushButton( tr("Edit"), hb1 );
	connect( edit_button, SIGNAL( clicked() ), this, SLOT( clickEditButton() ) );
	remove_button = new QPushButton( tr("Remove"), hb1 );
	connect( remove_button, SIGNAL( clicked() ), this, SLOT( clickRemoveButton() ) );
	remove_button = new QPushButton( tr("Hide"), hb1 );
	connect( remove_button, SIGNAL( clicked() ), this, SLOT( clickHideButton() ) );
	addIdentity = 0;
	editIdentity = 0;
	editPerson = 0;
}

PhoneBook::~PhoneBook( void )
{
}

QStringList PhoneBook::getRejectedContactsList()
{
	QListViewItem *item;
	QStringList sl;
	if( parser->getRejectedContacts() ) {
		item = parser->getRejectedContacts()->firstChild();
		while( item ) {
			sl.append( clean( item->text( 0 ) ).latin1() );
			item = item->nextSibling();
		}
	}
	return sl;
}

void PhoneBook::addContact( SipCallMember *member )
{
	bool personFound = false;
	bool identityFound = false;
	QString identity;
	QListViewItem *personItem;
	QListViewItem *identityItem = 0;

	if( !addIdentity ) {
		addIdentity = new PhoneBookAddIdentity( this, caption );
	}
	addIdentity->clean();
	addIdentity->setPerson( member->getUri().getFullname() );
	addIdentity->setIdentity( member->getUri().reqUri() );
	addIdentity->setDescription( "" );
	addIdentity->setContact( true );
	identity = addIdentity->getIdentity();
	if( identity.left( 4 ).lower() != "tel:" && identity.left( 4 ).lower() != "sip:" ) {
		identity = "sip:" + identity;
	}
	personItem = parser->getListPhoneBook()->firstChild();
	if( personItem ) {
		identityItem = personItem->firstChild();
		while( personItem ) {
			identityItem = personItem->firstChild();
			while( identityItem ) {
				if( identityItem->text( 0 ) == identity ) {
					identityFound = true;
					break;
				}
				identityItem = identityItem->nextSibling();
			}
			if( identityFound ) {
				personFound = true;
				addIdentity->setPerson( personItem->text( 0 ) );
				addIdentity->setDescription( identityItem->text( 2 ) );
				break;
			}
			personItem = personItem->nextSibling();
		}
	}
	addIdentity->exec();
	if( addIdentity->result() ) {
		if( !personFound ) {
			personItem = parser->getListPhoneBook()->firstChild();
			while( personItem ) {
				identityItem = personItem->firstChild();
				if( personItem->text( 0 ) == addIdentity->getPerson() ) {
					personFound = true;
					break;
				}
				personItem = personItem->nextSibling();
			}
		}
		if( !personFound ) {
			personItem = new QListViewItem( parser->getListPhoneBook(), addIdentity->getPerson(), "" );
		}
		personItem->setOpen( TRUE );
		if( !identityFound ) {
			new QListViewItem( personItem, identity, addIdentity->getContact(), addIdentity->getDescription() );
		} else {
			if( identityItem ) {
				identityItem->setText( 1, addIdentity->getContact() );
				identityItem->setText( 2, addIdentity->getDescription() );
			}
		}
	}
	save();
}

void PhoneBook::clickAddButton()
{
	bool done = false;
	QString s;
	QListViewItem *item;

	item = phoneBook->currentItem();
	if( !addIdentity ) {
		addIdentity = new PhoneBookAddIdentity( this, caption );
	}
	addIdentity->clean();
	if( item != 0 ) {
		if( item->parent() != 0 ) {
			if( item->parent()->parent() == 0 ) {
				if( item->parent()->text( 0 ) == "Permanently rejected contacts" ) {
					return;
				}
				if( item->parent()->text( 0 ) == "PhoneBook" ) {
					addIdentity->setPerson( item->text( 0 ) );
				}
			} else {
				if( item->parent()->parent()->parent() == 0 ) {
					if( item->parent()->parent()->text( 0 ) == "PhoneBook" ) {
						addIdentity->setPerson( item->parent()->text( 0 ) );
					}
				}
			}
		}
	}
	addIdentity->exec();
	if( addIdentity->result() ) {
		s = addIdentity->getIdentity();
		if( s.left( 4 ).lower() != "sip:" ) {
			s = "sip:" + s;
		}
		item = parser->getListPhoneBook()->firstChild();
		while( item ) {
			if( item->text( 0 ) == addIdentity->getPerson() ) {
				done = true;
				break;
			}
			item = item->nextSibling();
		}
		if( !done ) {
			item = new QListViewItem( parser->getListPhoneBook(), addIdentity->getPerson(), "" );
		}
		item->setOpen( TRUE );
		new QListViewItem( item, s, addIdentity->getContact(), addIdentity->getDescription() );
	}
	save();
}

void PhoneBook::clickEditButton()
{
	QString s;
	QListViewItem *item;
	item = phoneBook->currentItem();
	if( item != 0 ) {
		if( item->parent() != 0 ) {
			if( item->parent()->parent() == 0 ) {
				if( item->parent()->text( 0 ) == "PhoneBook" ) {
					if( !editPerson ) {
						editPerson = new PhoneBookEditPerson( this, caption );
					}
					editPerson->setPerson( item->text( 0 ) );
					editPerson->exec();
					if( editPerson->result() ) {
						item->setText( 0, editPerson->getPerson() );
					}
				}
			} else if( item->parent()->parent()->parent() == 0 ) {
				if( item->parent()->parent()->text( 0 ) == "PhoneBook" ) {
					if( !editIdentity ) {
						editIdentity = new PhoneBookEditIdentity( this, caption );
					}
					editIdentity->setIdentity( item->text( 0 ) );
					editIdentity->setContact( item->text( 1 ) );
					editIdentity->setDescription( item->text( 2 ) );
					editIdentity->exec();
					if( editIdentity->result() ) {
						s = editIdentity->getIdentity();
						if( s.left( 4 ).lower() != "sip:" ) {
							item->setText( 0, "sip:" + s );
						} else {
							item->setText( 0, s );
						}
						item->setText( 1, editIdentity->getContact() );
						item->setText( 2, editIdentity->getDescription() );
					}
				}
			}
			save();
		}
	} else {
//	PD	QMessageBox::information( this, tr("KPhone"), tr("Select first item.") );
	}
}

void PhoneBook::clickRemoveButton()
{
	bool OK = false;
	QListViewItem *item;
	item = phoneBook->currentItem();
	if( item != 0 ) {
		if( item->parent() != 0 ) {
			if( item->parent()->parent() == 0 ) {
				if( item->parent()->text( 0 ) == "Permanently rejected contacts" ) {
					OK = true;
				}
				if( item->parent()->text( 0 ) == "PhoneBook" ) {
					OK = true;
				}
			} else if( item->parent()->parent()->parent() == 0 ) {
				if( item->parent()->parent()->text( 0 ) == "PhoneBook" ) {
					OK = true;
				}
			} else if( item->parent()->parent()->parent()->parent() == 0 ) {
				if( item->parent()->parent()->parent()->text( 0 ) == "PhoneBook" ) {
					OK = true;
				}
			}
			if( OK ) {
//	PD			if( !QMessageBox::warning( this, tr("KPhone"), tr("Are You Sure ?"), tr("Yes"), tr("No"), "", 1) )
                //{
					delete item;
					item = 0;
					save();
				//}
			}
		}
	} else {
//	PD	QMessageBox::information( this, tr("KPhone"), tr("Select first item.") );
	}
}

void PhoneBook::clickHideButton()
{
	done(1);
}

void PhoneBook::save()
{
	QListViewItem *item1;
	QListViewItem *item2;

	FILE *f = fopen( filePhoneBook, "w" );
        if (f == NULL)
            return;
	item1 = parser->getListPhoneBook()->firstChild();
	fprintf( f, "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n" );
	fprintf( f, "<phonebook>\n" );
	while( item1 ) {
		fprintf( f, "    <person Name=\"%s\">\n", clean( item1->text( 0 ) ).latin1() );
		item2 = item1->firstChild();
		while( item2 ) {
			fprintf( f, "        <identity\n" );
			fprintf( f, "             uri = \"%s\"\n", clean( item2->text( 0 ) ).latin1() );
			fprintf( f, "             contact = \"%s\"\n", clean( item2->text( 1 ) ).latin1() );
			fprintf( f, "             description = \"%s\"\n", clean( item2->text( 2 ) ).latin1() );
			fprintf( f, "        />\n");
			item2 = item2->nextSibling();
		}
		fprintf( f, "    </person>\n" );
		item1 = item1->nextSibling();
	}
	fprintf( f, "</phonebook>\n" );
	fclose( f );
}

bool PhoneBook::setIdentity( QListViewItem *i )
{
	if( i != 0 ) {
		if( i->text( 0 ).left( 4 ).lower() == "sip:" ) {
			remote = i->parent()->text( 0 ) + ", " + i->text( 1 ) + " [" + i->text( 0 ) +"]";
			return true;
		}
		if( i->text( 0 ).mid( 4, 4 ).lower() == "sip:" ) {
			remote = i->parent()->text( 0 ) + ", " + i->text( 1 ) + " [" + i->text( 0 ).mid( 4 ) +"]";
			return true;
		}
	}
	return false;
}

void PhoneBook::setIdentityAndDone( QListViewItem *i )
{
	if (setIdentity( i ) ) {
		done(0);
	}
}

QString PhoneBook::getUri()
{
	return remote;
}

QString PhoneBook::clean( QString s )
{
	s.replace( QRegExp(">"), "" );
	s.replace( QRegExp("<"), "" );
	s.replace( QRegExp("\""), "" );
	s.replace( QRegExp("&"), "" );
	return s;
}
