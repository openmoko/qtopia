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
#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <qdglobal.h>
#include "qserialport.h"

class QD_EXPORT SerialPort : public QSerialPort
{
    Q_OBJECT
public:
    SerialPort( const QString &port );
    ~SerialPort();

    bool open( QIODevice::OpenMode mode );

signals:
    void newConnection();
    void disconnected();

private slots:
    void slotDsrChanged( bool dsrUp );
    void slotReadyRead();

private:
    bool up;
};

#endif
