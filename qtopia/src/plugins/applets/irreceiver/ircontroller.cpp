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

#include "ircontroller.h"

#include <qtopia/resource.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>

#include <qfile.h>
#include <qdir.h>

static const char* servicefile = "/var/lock/subsys/irda";

IRController::IRController(QObject* parent) : QObject(parent)
{
    // Load protocols
    QString tdir = QPEApplication::qpeDir()+"/etc/beam/targets";
    QDir dir(tdir,"*.conf");
    protocount=0;
    Config cfgout("Beam");
    cfgout.setGroup("Send");
    QString curdev=cfgout.readEntry("DeviceConfig");
    curproto = 0;
    for (int i=0; i<(int)dir.count(); i++) {
	QString t=tdir+"/"+dir[i];
	Config cfg(t,Config::File);
	if ( cfg.isValid() ) {
	    cfg.setGroup("Device");
	    names.append(cfg.readEntry("Name"));
	    icons.append(cfg.readEntry("Icon"));
	    targets.append(t);
	    if ( curdev.isEmpty() )
		curdev=t;
	    if ( curdev == t )
		curproto = protocount;
	    protocount++;
	}
    }
    if ( QFile::exists(servicefile) ) {
	st = On;
    } else {
	st = Off;
    }

    QCopChannel* obexChannel = new QCopChannel( "QPE/Obex", this );
    connect( obexChannel, SIGNAL(received(const QCString&,const QByteArray&)),
             this, SLOT(obexMessage(const QCString&,const QByteArray&)) );
}

void IRController::obexMessage(const QCString& msg, const QByteArray&)
{
    if ( msg == "turnedOff()") {
	if ( st != Off ) {
	    st = Off;
	    emit stateChanged(st);
	}
    }
}

IRController::State IRController::state() const
{
    return st;
}

int IRController::currentProtocol() const
{
    return curproto;
}

int IRController::protocolCount() const
{
    return protocount;
}

IRController::~IRController()
{
}

QString IRController::stateDescription(State s)
{
    switch ( s ) {
	case Off: return tr("Receiver off");
	case On: return tr("Receiver on");
	case On5Mins: return tr("On for 5 minutes");
	case On1Item: return tr("On for 1 item");
    }
    return QString::null;
}

QString IRController::protocolName(int i) const
{
    return names[i];
}

QIconSet IRController::protocolIcon(int i) const
{
    QString ic = icons[i];
    if ( !ic.isEmpty() )
	return Resource::loadIconSet(ic);
    return QIconSet();
}

bool IRController::setProtocol(int i)
{
    // read target config (validity test)
    QString dev = targets[i];
    Config cfgin(dev,Config::File);
    if ( cfgin.isValid() ) {
	// write Beam config
	Config cfgout("Beam");
	cfgout.setGroup("Send");
	cfgout.writeEntry("DeviceConfig",dev);
	return TRUE;
    } else {
	return FALSE;
    }
}

void IRController::setState(State s)
{
    st = s;
    switch (s) {
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
