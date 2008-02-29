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

#include "irreceiver.h"

#include <qpe/resource.h>

#include <qfile.h>
#include <qpopupmenu.h>

#include <stdlib.h>

const char* servicecmd = "/etc/rc.d/init.d/irda";
const char* servicefile = "/var/lock/subsys/irda";


//===========================================================================

IRReceiverApplet::IRReceiverApplet( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    QPopupMenu* menu = new QPopupMenu;
    it[0] = menu->insertItem(tr("Off"));
    it[1] = menu->insertItem(tr("On"));
    it[2] = menu->insertItem(tr("On for 5 minutes"));
    // NOT IMPLEMENTED
    // it[3] = menu->insertItem(tr("On for 1 item"));
    acc = new Accessory(Resource::loadIconSet("irreceive"),
	    tr("IR Receiver"), menu, this);
    connect(acc, SIGNAL(activated(int)), this, SLOT(activate(int)));
    tid = 0;
    setFixedWidth( 0 );
    if ( QFile::exists(servicefile) )
	activate(it[On]);
    else
	activate(it[Off]);
}

IRReceiverApplet::~IRReceiverApplet()
{
    if ( tid )
	service("off"); // No tr
}


void IRReceiverApplet::activate(int choice)
{
    for (int i=0; i<3; i++) {
	bool y = choice==it[i];
	acc->popup()->setItemChecked(it[i],y);
	if ( y ) {
	    if ( tid )
		killTimer(tid);
	    switch ((State)i) {

		case Off:
		    service("stop"); // No tr
		    break;
		case On:
		    service("start"); // No tr
		    break;
		case On5Mins:
		    service("start"); // No tr
		    tid = startTimer(5*60*1000);
		    break;
		/*
		case On1Item:
		    break;
		*/
	    }
	}
    }
}

void IRReceiverApplet::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == tid )
	activate(it[Off]);
}

void IRReceiverApplet::service(const QString& command)
{
    system((QString(servicecmd) + " " + command).latin1());
}
