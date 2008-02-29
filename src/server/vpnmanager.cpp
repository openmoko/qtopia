/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifdef QTOPIA_VPN

#include "vpnmanager.h"
#include "qtopiaserverapplication.h"

#include <qtopiacomm/private/qvpnmanager_p.h>

/*!
  \class QtopiaVpnManager
  \brief This class manages all virtual private network sessions regardless of their type
  */

//TODO merge QVpnManager and QtopiaVpnManager
QtopiaVpnManager::QtopiaVpnManager( QObject* parent )
    : QObject( parent )
{
    manager = new QVpnManager( this );
}


QtopiaVpnManager::~QtopiaVpnManager()
{
}

QTOPIA_TASK(QtopiaVpnManager,QtopiaVpnManager);

#endif
