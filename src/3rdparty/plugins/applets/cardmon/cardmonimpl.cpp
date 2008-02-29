/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS and its licensors.
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
#include <qapplication.h>
#include <qpixmap.h>
#include "cardmon.h"
#include "cardmonimpl.h"


CardMonitorImpl::CardMonitorImpl()
    : cardMonitor(0), ref(0)
{
}

CardMonitorImpl::~CardMonitorImpl()
{
    delete cardMonitor;
}

QWidget *CardMonitorImpl::applet( QWidget *parent )
{
    if ( !cardMonitor )
	cardMonitor = new CardMonitor( parent );
    return cardMonitor;
}

int CardMonitorImpl::position() const
{
    return 0;
}

QRESULT CardMonitorImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_TaskbarApplet )
	*iface = this;

    if ( *iface )
	(*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( CardMonitorImpl )
}
