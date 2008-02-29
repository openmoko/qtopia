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

#ifndef __QBLUETOOHRFCOMMSERIALPORT__H
#define __QBLUETOOHRFCOMMSERIALPORT__H

#include <QObject>
#include <QString>
#include <QStringList>

#include "qbluetoothaddress.h"
#include "qbluetoothrfcommsocket.h"

class QBluetoothRfcommSerialPortPrivate;

class QTOPIACOMM_EXPORT QBluetoothRfcommSerialPort : public QObject
{
    Q_OBJECT
public:
    explicit QBluetoothRfcommSerialPort(QObject* parent = 0);
    ~QBluetoothRfcommSerialPort();

    QString createTty( QBluetoothRfcommSocket* socket );
    QString createTty( const QBluetoothAddress& local, const QBluetoothAddress &remote, int channel);
    void releaseTty();
    QString boundDevice() const;

    static QStringList listBindings();

private:
    QBluetoothRfcommSerialPortPrivate* d;
};

#endif //__QBLUETOOHRFCOMMSERIALPORT__H
