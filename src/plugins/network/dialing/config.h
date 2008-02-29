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
#ifndef DIALUPCONFIGIMPL_H
#define DIALUPCONFIGIMPL_H

#include <qtopianetworkinterface.h>
#include <qtranslatablesettings.h>

#include <QDialog>
class DialupUI;

class DialupConfig : public QtopiaNetworkConfiguration
{
public:
    DialupConfig( const QString& confFile );
    virtual ~DialupConfig();

    virtual QString configFile() const
    {
        return currentConfig;
    }

    virtual QStringList types() const;
    virtual QDialog* configure( QWidget* parent, const QString& type = QString() );

    virtual QVariant property( const QString& key ) const;
    virtual QtopiaNetworkProperties getProperties() const;
    virtual void writeProperties( const QtopiaNetworkProperties& properties );

private:
    QString currentConfig;
    mutable QTranslatableSettings cfg;
};

#endif
