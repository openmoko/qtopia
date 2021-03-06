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

#ifndef BLUETOOTHPLUGIN_H
#define BLUETOOTHPLUGIN_H

#include <qtopianetworkinterface.h>
#include <qtopiaglobal.h>

#include <QByteArray>
#include <QList>
#include <QPointer>

//#include "bluetooth.h"

class QTOPIA_PLUGIN_EXPORT BluetoothPlugin : public QtopiaNetworkPlugin
{
public:
    BluetoothPlugin();
    virtual ~BluetoothPlugin();

    virtual QPointer<QtopiaNetworkInterface> network( const QString& confFile);
    virtual QtopiaNetwork::Type type() const;
    virtual QByteArray customID() const;
private:
    QList<QPointer<QtopiaNetworkInterface> > instances;
};

#endif //DialupPlugin_H
