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

#ifndef _CELLMODEMMANAGERTEST_H_
#define _CELLMODEMMANAGERTEST_H_

#include <QObject>
#include "phone/cellmodemmanager.h"

class CellModemManagerTest : public QObject
{
Q_OBJECT
public:
    CellModemManagerTest(QObject *parent = 0);

private slots:
    void received(const QString &, const QByteArray &);
    void planeModeEnabledChanged(bool);
    void registrationStateChanged(QTelephony::RegistrationState);
    void networkOperatorChanged(const QString &);
    void cellLocationChanged(const QString &);
    void stateChanged(CellModemManager::State newState,
                      CellModemManager::State oldState);
    void callForwardingEnabledChanged(bool);
    void simToolkitAvailableChanged(bool);

private:
    CellModemManager *d;
};

#endif // _CELLMODEMMANAGERTEST_H_
