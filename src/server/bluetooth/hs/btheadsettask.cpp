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

#ifdef HAVE_ALSA

#include "btheadsettask.h"
#include "qtopiaserverapplication.h"
#include "qbluetoothhsservice_p.h"
#include <qtopialog.h>

BtHeadsetService::BtHeadsetService( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "Initializing Headset Service";
    m_hsService = new QBluetoothHeadsetService( "BluetoothHeadset", this );
    m_hsService->initialize();
}

BtHeadsetService::~BtHeadsetService()
{
    delete m_hsService;
}

QTOPIA_TASK( BtHeadsetService, BtHeadsetService );

#endif
