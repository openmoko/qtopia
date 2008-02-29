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

#include "btdialupservice.h"

#include <qtopianamespace.h>
#include <QFile>

/*!
  \class BtDialupServiceTask
  \brief The BtDialupServiceTask class provides server side support for the Bluetooth 
  DUN profile.
  \ingroup QtopiaServer::Task::Bluetooth

  This task listens for incoming Bluetooth DUN connections, forwards the request to 
  Qtopia's modem emulator and manages the life time of these connections. This task relies 
  on QBluetoothSerialPortService.
  
  The BtDialupServiceTask class provides the \c {BtDialupServiceTask} task.
  This class is part of the Qtopia server and cannot be used by other QtopiaApplications.

  \sa QBluetoothSerialPortService
  */

/*!
  Constructs the BtDialupServiceTask instance with the given \a parent.
  */
BtDialupServiceTask::BtDialupServiceTask( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "Initializing Bluetooth DialupService";

    provider = new QBluetoothSerialPortService( QLatin1String("DialupNetworking"),
            tr("Dial-up Networking"),
            Qtopia::qtopiaDir() + "etc/bluetooth/sdp/dun.xml",
            2,      // the channel number stored in dun.xml
            this );
}

/*!
  Destroys the BtDialupServiceTask instance.
  */
BtDialupServiceTask::~BtDialupServiceTask()
{
}

QTOPIA_TASK(BtDialupServiceTask,BtDialupServiceTask);
#include "btdialupservice.moc"

