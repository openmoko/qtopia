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

#ifndef BT_DIALUP_SERVICE_H
#define BT_DIALUP_SERVICE_H

#include <QObject>
#include <QString>

#include "../bluetoothserialportservice.h"
#include <qtopiaserverapplication.h>

class QBluetoothSerialPortService;
class BtDialupServiceTask : public QObject
{
    Q_OBJECT
public:
    BtDialupServiceTask( QObject* parent = 0 );
    ~BtDialupServiceTask();

private:
    QBluetoothSerialPortService* provider;
};

#endif //BT_DIALUP_SERVICE_h
