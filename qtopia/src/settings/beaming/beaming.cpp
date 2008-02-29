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
#include "beaming.h"
#include "../../plugins/applets/irreceiver/ircontroller.h"

#include <qsignalmapper.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qtimer.h>
#include <qlistbox.h>

#include <qtopia/global.h>

static int AUTO_QUIT_TIME = 650; // or 0 if auto-quitting is confusing.

Beaming::Beaming( QWidget* parent,  const char* name, WFlags fl )
    : QVBox( parent, name, TRUE, fl )
{
    setCaption(tr("Beaming"));
    setMargin(4);
    setSpacing(2);

    irc = new IRController(this);

    QButtonGroup* bg = new QButtonGroup(this);
    bg->hide();
    QSignalMapper* sm = new QSignalMapper(this);
    for (int i=0; i<=(int)IRController::LastState; i++) {
	QRadioButton *b = new QRadioButton(this);
	bg->insert(b);
	b->setText(IRController::stateDescription((IRController::State)i));
	if ( (IRController::State)i == irc->state() )
	    b->setChecked(TRUE);
	if ( irc->state() == IRController::Off ) {
	    if ( (IRController::State)i == IRController::On1Item )
		b->setFocus();
	} else {
	    if ( (IRController::State)i == IRController::Off )
		b->setFocus();
	}
	connect(b,SIGNAL(clicked()), sm, SLOT(map()));
	sm->setMapping(b,i);
    }
    connect(sm,SIGNAL(mapped(int)),this,SLOT(chooseState(int)));

    int pc = irc->protocolCount();
    if ( pc ) {
	lb = new QListBox(this);
	for (int i=0; i<pc; i++) {
	    QString n = irc->protocolName(i);
	    QIconSet ic = irc->protocolIcon(i);
	    if ( ic.pixmap().isNull() )
		lb->insertItem(n,i);
	    else
		lb->insertItem(ic.pixmap(),n,i);
	}
	lb->setSelected(irc->currentProtocol(),TRUE);
	connect(lb,SIGNAL(selected(int)),this,SLOT(chooseProtocol(int)));
    }

#ifdef QTOPIA_PHONE
    // add context menu for help
    contextMenu = new ContextMenu( this );
#endif
}

void Beaming::chooseState(int c)
{
    irc->setState((IRController::State)c);
    if ( AUTO_QUIT_TIME )
	QTimer::singleShot( AUTO_QUIT_TIME, this, SLOT(close()) );
}

void Beaming::chooseProtocol(int c)
{
    irc->setProtocol(c);
#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() )
	lb->setModalEditing(FALSE);
#endif
}

Beaming::~Beaming()
{
}
