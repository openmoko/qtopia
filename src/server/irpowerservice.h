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

#ifndef __IRPOWERSERVICE_H__
#define __IRPOWERSERVICE_H__

#include "qabstractdevicemanager.h"

class IrPowerService_Private;
class IrPowerService : public QAbstractCommDeviceManager
{
    Q_OBJECT
public:
    IrPowerService(const QByteArray &serverPath,
                   const QByteArray &devId,
                   QObject *parent = 0);
    ~IrPowerService();

    virtual void bringUp();
    virtual void bringDown();
    virtual bool isUp() const;
    virtual bool shouldBringDown(QUnixSocket *) const;

private:
    IrPowerService_Private *m_data;
};

class QValueSpaceObject;

class IrPowerServiceTask : public QObject
{
Q_OBJECT
public:
    IrPowerServiceTask(QObject *parent = 0);
    ~IrPowerServiceTask();

private:
    IrPowerService *m_irPower;
    QValueSpaceObject *m_valueSpace;
};

#endif
