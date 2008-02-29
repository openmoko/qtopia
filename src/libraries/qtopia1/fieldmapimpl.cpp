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

#include <qtopia/fieldmapimpl.h>

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qvbox.h>

/*!
  \class FieldMap fieldmapimpl.h

  \internal
*/

FieldMap::FieldMap(QWidget *parent, const char* name)
    : QWidget(parent, name)
{
    init();
    
    connect( addButton, SIGNAL(clicked()), this, SLOT( addClicked() ) ); 
    connect( fieldBox, SIGNAL(doubleClicked(QListBoxItem *)), this, SLOT(addClicked()) );

    connect( removeButton, SIGNAL(clicked()), this, SLOT( removeClicked() ) ); 
    connect( displayBox, SIGNAL(doubleClicked(QListBoxItem *)), this, SLOT(removeClicked()) );

    connect( moveUpButton, SIGNAL(clicked()), this, SLOT( moveUp() ) );
    connect( moveDownButton, SIGNAL(clicked()), this, SLOT( moveDown() ) );

    displayBox->setSelectionMode( QListBox::Multi );
    fieldBox->setSelectionMode( QListBox::Multi );
}

void FieldMap::init()
{
    setCaption( tr( "Header Fields" ) );

    QGridLayout *grid = new QGridLayout( this ); 
    grid->setSpacing( 3 );
    grid->setMargin( 4 );

    QGridLayout *Layout5 = new QGridLayout; 
    Layout5->setSpacing( 3 );
    Layout5->setMargin( 0 );

    QLabel *label = new QLabel( this );
    label->setText( tr( "Displayed" ) );
    Layout5->addWidget( label, 0, 2 );

    addButton = new QPushButton( this, "addButton" );
    addButton->setText( tr( "->" ) );
    QWhatsThis::add(  addButton, tr( "Display the selected column" ) );
    Layout5->addWidget( addButton, 2, 1 );

    label = new QLabel( this);
    label->setText( tr( "Available" ) );
    Layout5->addWidget( label, 0, 0 );

    displayBox = new QListBox( this, "displayBox" );
    QWhatsThis::add(  displayBox, tr( "A list of the columns currently being displayed" ) );
    Layout5->addMultiCellWidget( displayBox, 1, 4, 2, 2 );

    fieldBox = new QListBox( this, "fieldBox" );
    QWhatsThis::add(  fieldBox, tr( "A list of the columns available for display" ) );
    Layout5->addMultiCellWidget( fieldBox, 1, 4, 0, 0 );
    
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout5->addItem( spacer, 4, 1 );

    removeButton = new QPushButton( this, "removeButton" );
    removeButton->setText( tr( "<-" ) );
    QWhatsThis::add(  removeButton, tr( "Remove the selected column from display" ) );
    Layout5->addWidget( removeButton, 3, 1 );
    
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout5->addItem( spacer_2, 1, 1 );

    grid->addMultiCellLayout( Layout5, 0, 0, 0, 1 );

    QVBoxLayout *Layout6 = new QVBoxLayout; 
    Layout6->setSpacing( 3 );
    Layout6->setMargin( 0 );

    moveUpButton = new QPushButton( this, "moveUpButton" );
    moveUpButton->setText( tr( "Move up" ) );
    QWhatsThis::add(  moveUpButton, tr( "" ) );
    Layout6->addWidget( moveUpButton );

    moveDownButton = new QPushButton( this, "moveDownButton" );
    moveDownButton->setText( tr( "Move down" ) );
    Layout6->addWidget( moveDownButton );

    grid->addLayout( Layout6, 1, 1 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    grid->addItem( spacer_3, 1, 0 );
}

void FieldMap::setFields( const QMap<int,QString> &all, const QValueList<int> &display)
{
    displayBox->clear();
    fieldBox->clear();
    keys.clear();

    for ( QMap<int,QString>::ConstIterator it = all.begin(); it != all.end(); ++it) {
	if ( display.find( it.key() ) == display.end() )
	    fieldBox->insertItem( (*it) );

	keys.insert( *it, it.key() );
    }

    // These fields must be inserted in correct order
    for ( QValueList<int>::ConstIterator e = display.begin(); e != display.end(); ++e) {
	displayBox->insertItem( all[*e] );
    }
}

QValueList<int> FieldMap::fields()
{
    QValueList<int> l;

    QListBoxItem *item = displayBox->firstItem();
    while ( item ) {
	l.append( keys[ item->text() ] );
	item  = item->next();
    }
    
    return l;
}

void FieldMap::addClicked()
{
    QListBoxItem *item = fieldBox->firstItem();
    while ( item ) {
	if ( item->selected() ) {
	    displayBox->insertItem( item->text() );
	    QListBoxItem *temp = item;
	    item = item->next();
	    delete temp;
	} else {
	    item = item->next();
	}
    }
}

void FieldMap::removeClicked()
{
    QListBoxItem *item = displayBox->firstItem();
    while ( item ) {
	if ( item->selected() ) {
	    fieldBox->insertItem( item->text() );
	    QListBoxItem *temp = item;
	    item = item->next();
	    delete temp;
	} else {
	    item = item->next();
	}
    }
}

void FieldMap::moveUp()
{
    QListBoxItem *item = displayBox->firstItem();
    while ( item ) {
	if ( item->selected() ) {
	    QListBoxItem *prev = item->prev();
	    if ( prev && !prev->selected() ) {
		int pos = displayBox->index(prev);
		displayBox->insertItem(item->text(), pos );
		displayBox->setSelected(pos, TRUE );
		delete item;
		item = displayBox->item( pos+1 );
	    } else {
	    	item = item->next();
	    }
	} else {
	    item = item->next();
	}
    }
}

void FieldMap::moveDown()
{
    QListBoxItem *item = displayBox->item( displayBox->count() - 1 );
    while ( item ) {
	if ( item->selected() ) {
	    QListBoxItem *next = item->next();
	    if ( next && !next->selected() ) {
		int pos = displayBox->index(next);
		pos++;
		displayBox->insertItem(item->text(), pos );
		displayBox->setSelected(pos, TRUE );
		delete item;
		item = displayBox->item( pos-2 );
	    } else {
	    	item = item->prev();
	    }
	} else {
	    item = item->prev();
	}
    }
}

