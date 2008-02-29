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
  \brief The FieldMap class provides a UI for selecting an
	ordered list of fields from a set of available fields.

  First availability: Qtopia 1.6

  \internal
*/

FieldMap::FieldMap(QWidget *parent, const char* name)
    : QWidget(parent, name)
{
    init();
    
    connect( addButton, SIGNAL(clicked()), this, SLOT( addClicked() ) ); 
    connect( fieldBox, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(addClicked()) );

    connect( removeButton, SIGNAL(clicked()), this, SLOT( removeClicked() ) ); 
    connect( displayBox, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(removeClicked()) );

    connect( moveUpButton, SIGNAL(clicked()), this, SLOT( moveUp() ) );
    connect( moveDownButton, SIGNAL(clicked()), this, SLOT( moveDown() ) );

    displayBox->setSelectionMode( QListBox::Multi );
    fieldBox->setSelectionMode( QListBox::Multi );

    connect(displayBox, SIGNAL(selectionChanged()),
	    this, SLOT(enableButtons()) );
    connect(fieldBox, SIGNAL(selectionChanged()),
	    this, SLOT(enableButtons()) );
    enableButtons();
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
    Layout6->addWidget( moveUpButton );

    moveDownButton = new QPushButton( this, "moveDownButton" );
    moveDownButton->setText( tr( "Move down" ) );
    Layout6->addWidget( moveDownButton );

    grid->addLayout( Layout6, 1, 1 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    grid->addItem( spacer_3, 1, 0 );

    setTabOrder(fieldBox, addButton);
    setTabOrder(addButton, displayBox);
    setTabOrder(displayBox, removeButton);
    setTabOrder(removeButton, moveUpButton);
    setTabOrder(moveUpButton, moveDownButton);
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
    enableButtons();
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
    enableButtons();
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
    enableButtons();
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

void FieldMap::enableButtons()
{
    addButton->setEnabled( containsSelection(fieldBox) );
    bool enable = containsSelection(displayBox);
    removeButton->setEnabled( enable );
    moveUpButton->setEnabled( enable );
    moveDownButton->setEnabled( enable );
}

bool FieldMap::containsSelection(QListBox *b)
{
    QListBoxItem *item = b->firstItem();
    while ( item ) {
	if ( item->selected() )
	    return TRUE;
	item = item->next();
    }
    
    return FALSE;
}

