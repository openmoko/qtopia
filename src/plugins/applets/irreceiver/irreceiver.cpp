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

#include <qtopia/resource.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>

#include <qfile.h>
#include <qdir.h>
#include <qpopupmenu.h>

static const char* servicefile = "/var/lock/subsys/irda";
static const int protobase=100;


//===========================================================================

IRReceiverApplet::IRReceiverApplet( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    QPopupMenu* menu = new QPopupMenu;
    it[0] = menu->insertItem(tr("Receiver off"));
    it[1] = menu->insertItem(tr("Receiver on"));
    it[2] = menu->insertItem(tr("On for 5 minutes"));
    it[3] = menu->insertItem(tr("On for 1 item"));

    // Load protocols
    QString tdir = QPEApplication::qpeDir()+"/etc/beam/targets";
    QDir dir(tdir,"*.conf");
    protocount=0;
    Config cfgout("Beam");
    cfgout.setGroup("Send");
    QString curdev=cfgout.readEntry("Device");
    for (int i=0; i<(int)dir.count(); i++) {
	QString t=tdir+"/"+dir[i];
	Config cfg(t,Config::File);
	if ( cfg.isValid() ) {
	    cfg.setGroup("Device");
	    QString n = cfg.readEntry("Name");
	    QString ic = cfg.readEntry("Icon");
	    if ( !protocount ) {
		menu->insertSeparator();
		if ( curdev.isEmpty() )
		    curdev=t;
	    }
	    QIconSet icon;
	    if ( !ic.isEmpty() )
		icon = Resource::loadIconSet(ic);
	    if ( icon.pixmap().isNull() )
		menu->insertItem(n,protobase+protocount);
	    else
		menu->insertItem(icon,n,protobase+protocount);
	    if ( t==curdev )
		menu->setItemChecked(protobase+protocount,TRUE);
	    protocount++;
	    targets.append(t);
	}
    }
    
    acc = new Accessory(Resource::loadIconSet("irreceive"),
	    tr("Beaming"), menu, this);
    connect(acc, SIGNAL(activated(int)), this, SLOT(activate(int)));
    setFixedWidth( 0 );
    
    if ( QFile::exists(servicefile) ) {
	state = On;
	acc->popup()->setItemChecked(it[(int) On],TRUE);
    } else {
	state = Off;
	acc->popup()->setItemChecked(it[(int) Off],TRUE);
    }

    QCopChannel* obexChannel = new QCopChannel( "QPE/Obex", this );
    connect( obexChannel, SIGNAL(received(const QCString&, const QByteArray&)),
             this, SLOT(obexMessage(const QCString&, const QByteArray&)) );
}

IRReceiverApplet::~IRReceiverApplet()
{
}

void IRReceiverApplet::obexMessage(const QCString& msg, const QByteArray&)
{
    if ( msg == "turnedOff()") {
	if ( state != Off ) {
	    state = Off;
	    for (int i=0; i<4; i++) {
		bool y = state==i;
		acc->popup()->setItemChecked(it[i],y);
	    }
	}
    }
}

void IRReceiverApplet::activate(int choice)
{
    QPopupMenu* menu = acc->popup();
    if ( choice >= protobase ) {
	// read target config
	QString dev = targets[choice-protobase];
	Config cfgin(dev,Config::File);
	if ( cfgin.isValid() ) {
	    // write Beam config
	    Config cfgout("Beam");
	    cfgout.setGroup("Send");
	    cfgout.writeEntry("DeviceConfig",dev);
	} else {
	    choice = -1;
	}
	for (int i=protobase; i-protobase<protocount; i++)
	    menu->setItemChecked(i,i==choice);
    } else {
	for (int i=0; i<4; i++) {
	    bool y = choice==it[i];
	    menu->setItemChecked(it[i],y);
	    if ( y ) {
		state = (State)i;
		switch ((State)i) {

		    case Off:
			QCopEnvelope("QPE/Obex", "turnOff()");
			break;
		    case On:
			QCopEnvelope("QPE/Obex", "turnOn()");
			break;
		    case On5Mins:
			{
			    int t = 5;  // 5 minutes
			    QCopEnvelope("QPE/Obex", "turnOnTimed(int)")
				<< t;
			    break;
			}
		    case On1Item:
			QCopEnvelope("QPE/Obex", "turnOn1Item()");
			break;
		}
	    }
	}
    }
}

