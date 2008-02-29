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

#include <custom.h>

#ifndef NO_WIRELESS_LAN

#include <QWlanRegistration>
#include <QAbstractIpcInterfaceGroup>

class WlanRegistrationInterface;
class WlanRegistrationProvider : public QAbstractIpcInterfaceGroup
{
    Q_OBJECT
public:
    WlanRegistrationProvider( const QString& serviceName, QObject* parent = 0 );
    ~WlanRegistrationProvider();

    void initialize();

    void setAccessPoint( const QString& essid = QString() );
    void notifyClients();

private:
    WlanRegistrationInterface* wri;
    QString servName;
    QString essid;
};

#endif //NO_WIRELESS_LAN
