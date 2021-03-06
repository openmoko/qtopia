/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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
#ifdef QTOPIA_CELL

#include <QObject>
class QFileSystemWatcher;
class QNetworkDevice;
class QNetworkRegistration;
class QValueSpaceObject;
class QCommServiceManager;
class GPRSMonitor : public QObject
{
    Q_OBJECT
public:
    GPRSMonitor( QObject* parent = 0 );
    ~GPRSMonitor();
private slots:
    void dataAccountsChanged();
    void gprsStateChanged();
    void currentOperatorChanged();
    void servicesAdded();

private:
    QFileSystemWatcher* watcher;
    QList<QNetworkDevice*> knownGPRSDevices;
    QValueSpaceObject* vso;
    QNetworkRegistration* netReg;
    QCommServiceManager* commManager;
    bool umts;
};
#endif
