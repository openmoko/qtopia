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

#include <qapplication.h>
#include "irreceiver.h"
#include "irreceiverappletimpl.h"


IRReceiverAppletImpl::IRReceiverAppletImpl()
    : irreceiver(0), icn(0), ref(0)
{
}

IRReceiverAppletImpl::~IRReceiverAppletImpl()
{
    delete irreceiver;
    delete icn;
}

QWidget *IRReceiverAppletImpl::applet( QWidget *parent )
{
    if ( !irreceiver )
	irreceiver = new IRReceiverApplet( parent );
    return irreceiver;
}

int IRReceiverAppletImpl::position() const
{
    return 6;
}

QString IRReceiverAppletImpl::name() const
{
    return qApp->translate( "Taskbar Applet", "IR Receiver" );
}

QPixmap IRReceiverAppletImpl::icon() const
{
    if ( !icn ) {
	IRReceiverAppletImpl *that = (IRReceiverAppletImpl*)this;
	that->icn = new QPixmap();
    }
    return *icn;
}

QRESULT IRReceiverAppletImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_TaskbarApplet )
	*iface = this;
    else if ( uuid == IID_TaskbarNamedApplet )
	*iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( IRReceiverAppletImpl )
}


