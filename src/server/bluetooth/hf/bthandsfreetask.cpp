/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "bthandsfreetask.h"
#include "qtopiaserverapplication.h"
#include "qbluetoothhfservice_p.h"
#include "btaudiovolumemanager.h"
#include <qtopialog.h>

/*!
    \class BtHandsfreeServiceTask
    \ingroup QtopiaServer::Task::Bluetooth
    \brief The BtHandsfreeServiceTask class provides an implementation of the Bluetooth Handsfree Service.

    The BtHandsfreeServiceTask manages the lifetime of a
    QBluetoothHandsfreeService object.
  
    This class is part of the Qtopia server and cannot be used by other QtopiaApplications.
 */

/*!
    Create a BtHandsfreeService task with \a parent.
*/
BtHandsfreeServiceTask::BtHandsfreeServiceTask( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "Initializing Handsfree Service";
    m_hfService = new QBluetoothHandsfreeService( "BluetoothHandsfree", tr("Handsfree Audio Gateway"), this );
    new BtAudioVolumeManager( "BluetoothHandsfree", this );
}

/*!
    Destructor.
*/
BtHandsfreeServiceTask::~BtHandsfreeServiceTask()
{
    delete m_hfService;
}

QTOPIA_TASK( BtHandsfreeServiceTask, BtHandsfreeServiceTask );
