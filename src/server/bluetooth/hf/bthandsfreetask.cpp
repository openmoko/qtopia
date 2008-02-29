/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "bthandsfreetask.h"
#include "qtopiaserverapplication.h"
#include "qbluetoothhfservice_p.h"
#include <qtopialog.h>

BtHandsfreeService::BtHandsfreeService( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "Initializing Handsfree Service";
    m_hfService = new QBluetoothHandsfreeService( "BluetoothHandsfree", this );
    m_hfService->initialize();
}

BtHandsfreeService::~BtHandsfreeService()
{
    delete m_hfService;
}

QTOPIA_TASK( BtHandsfreeService, BtHandsfreeService );
