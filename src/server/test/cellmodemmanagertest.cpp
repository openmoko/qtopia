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

#include "cellmodemmanagertest.h"
#include "qtopiaserverapplication.h"
#include <qcopchannel_qws.h>
#include <QtopiaIpcEnvelope>

CellModemManagerTest::CellModemManagerTest(QObject *parent)
: QObject(parent)
{
    d = qtopiaTask<CellModemManager>();
    QCopChannel *channel = new QCopChannel("Test/CellModemManager", this);
    QObject::connect(channel, SIGNAL(received(QString,QByteArray)),
                     this, SLOT(received(QString,QByteArray)));

    QObject::connect(d, SIGNAL(planeModeEnabledChanged(bool)), this, SLOT(planeModeEnabledChanged(bool)));
    QObject::connect(d, SIGNAL(registrationStateChanged(QTelephony::RegistrationState)), this, SLOT(registrationStateChanged(QTelephony::RegistrationState)));
    QObject::connect(d, SIGNAL(networkOperatorChanged(QString)), this, SLOT(networkOperatorChanged(QString)));
    QObject::connect(d, SIGNAL(cellLocationChanged(QString)), this, SLOT(cellLocationChanged(QString)));
    QObject::connect(d, SIGNAL(stateChanged(CellModemManager::State,CellModemManager::State)), this, SLOT(stateChanged(CellModemManager::State,CellModemManager::State)));
    QObject::connect(d, SIGNAL(callForwardingEnabledChanged(bool)), this, SLOT(callForwardingEnabledChanged(bool)));
    QObject::connect(d, SIGNAL(simToolkitAvailableChanged(bool)), this, SLOT(simToolkitAvailableChanged(bool)));
}

void CellModemManagerTest::received(const QString &msg, const QByteArray &data)
{
    QDataStream ds(data);
    if(msg == "blockProfiles()") {
        d->blockProfiles(true);
    } else if(msg == "unblockProfiles()") {
        d->blockProfiles(false);
    } else if(msg == "enablePlaneMode()") {
        d->setPlaneModeEnabled(true);
    } else if(msg == "disablePlaneMode()") {
        d->setPlaneModeEnabled(false);
    } else if(msg == "setSimPin(QString)") {
        QString pin;
        ds >> pin;
        d->setSimPin(pin);
    } else if(msg == "setSimPuk(QString,QString)") {
        QString puk, newpin;
        ds >> puk >> newpin;
        d->setSimPuk(puk, newpin);
    }
}

void CellModemManagerTest::planeModeEnabledChanged(bool enabled)
{
    QtopiaIpcEnvelope e("Test/CellModemManager", "planeModeEnabledChanged(bool)");
    e << enabled;
}

void CellModemManagerTest::registrationStateChanged(QTelephony::RegistrationState state)
{
    QtopiaIpcEnvelope e("Test/CellModemManager", "registrationStateChanged(QString)");
    switch(state) {
        case QTelephony::RegistrationNone:
            e << QString("None");
            break;
        case QTelephony::RegistrationHome:
            e << QString("Home");
            break;
        case QTelephony::RegistrationSearching:
            e << QString("Searching");
            break;
        case QTelephony::RegistrationDenied:
            e << QString("Denied");
            break;
        case QTelephony::RegistrationUnknown:
            e << QString("Unknown");
            break;
        case QTelephony::RegistrationRoaming:
            e << QString("Roaming");
            break;
        default:
            e << QString("Invalid");
            break;
    }
}

void CellModemManagerTest::networkOperatorChanged(const QString &op)
{
    QtopiaIpcEnvelope e("Test/CellModemManager", "networkOperatorChanged(QString)");
    e << op;
}

void CellModemManagerTest::cellLocationChanged(const QString &loc)
{
    QtopiaIpcEnvelope e("Test/CellModemManager", "cellLocationChanged(QString)");
    e << loc;
}

void CellModemManagerTest::stateChanged(CellModemManager::State newState,
                                        CellModemManager::State oldState)
{
    QtopiaIpcEnvelope e("Test/CellModemManager", "stateChanged(QString,QString)");
    e << CellModemManager::stateToString(newState) << CellModemManager::stateToString(oldState);
}

void CellModemManagerTest::callForwardingEnabledChanged(bool enabled)
{
    QtopiaIpcEnvelope e("Test/CellModemManager", "callForwardingEnabledChanged(bool)");
    e << enabled;
}

void CellModemManagerTest::simToolkitAvailableChanged(bool enabled)
{
    QtopiaIpcEnvelope e("Test/CellModemManager", "simToolkitAvailableChanged(bool)");
    e << enabled;
}

QTOPIA_TASK(CellModemManagerTest, CellModemManagerTest);
