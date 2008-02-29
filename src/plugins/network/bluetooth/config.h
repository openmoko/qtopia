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
#ifndef BLUETOOTHCONFIGIMPL_H
#define BLUETOOTHCONFIGIMPL_H

#include <qtopianetworkinterface.h>
#include <qtranslatablesettings.h>

#include <QDialog>

class BluetoothConfig : public QtopiaNetworkConfiguration
{
public:
    BluetoothConfig( const QString& confFile );
    virtual ~BluetoothConfig();

    virtual QString configFile() const
    {
        return currentConfig;
    }

    QStringList types() const;

    virtual QVariant property( const QString& key ) const;

    virtual QDialog* configure( QWidget* parent, const QString& type = QString() );
    virtual QtopiaNetworkProperties getProperties() const;
    virtual void writeProperties( const QtopiaNetworkProperties& properties );

private:
    QString currentConfig;
    mutable QTranslatableSettings cfg;
};

#endif
