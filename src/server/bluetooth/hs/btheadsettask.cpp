/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include "btheadsettask.h"
#include "qtopiaserverapplication.h"
#include "qbluetoothhsservice_p.h"
#include "bluetooth/btaudiovolumemanager_p.h"
#include <qtopialog.h>

/*!
    \class BtHeadsetServiceTask
    \ingroup QtopiaServer::Task::Bluetooth
    \brief The BtHeadsetServiceTask class provides the Bluetooth Headset Service.

    The BtHeadsetService task manages the lifetime of a
    QBluetoothHeadsetService object.
  
    This class is part of the Qtopia server and cannot be used by other QtopiaApplications.
 */

/*!
    Create a BtHeadsetService task with \a parent.
*/
BtHeadsetServiceTask::BtHeadsetServiceTask( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "Initializing Headset Service";
    m_hsService = new QBluetoothHeadsetService( "BluetoothHeadset", tr("Headset Audio Gateway"), this );
    new BtAudioVolumeManager( "BluetoothHeadset", this );
}

/*!
    Destructor.
*/
BtHeadsetServiceTask::~BtHeadsetServiceTask()
{
    delete m_hsService;
}

QTOPIA_TASK( BtHeadsetServiceTask, BtHeadsetServiceTask );
