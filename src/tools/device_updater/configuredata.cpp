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

#include "configuredata.h"

#include <QSettings>

#define ORG_STRING "com.trolltech.qtopia"
#define APP_STRING "device_updater"

/*!
  \class ConfigureData
  \brief Auto-saving Data Transfer Object (DTO) for configuration date.

  This simple class serves as a container to allow passing the configuration data
  around between classes.

  Use the load() and save() methods to retrieve or store the configuration to a
  settings file, to persist between runs.
*/

ConfigureData::ConfigureData()
    : mDirty( false )
{
}

ConfigureData::ConfigureData( const ConfigureData &rhs )
{
    this->mServer = rhs.mServer;
    this->mPort = rhs.mPort;
    this->mCommand = rhs.mCommand;
    this->mDocRoot = rhs.mDocRoot;
    this->mDirty = rhs.mDirty;
}

ConfigureData::~ConfigureData()
{
}

void ConfigureData::load()
{
    QSettings conf( ORG_STRING, APP_STRING );
    mServer = conf.value( "listen/address", "10.10.10.21" ).toString();
    mPort = (quint64)( conf.value( "listen/port", QVariant( 8080 )).toInt() );
    mCommand = conf.value( "package/command", "mkPackages" ).toString();
    mDocRoot = conf.value( "package/documentRoot", "." ).toString();
    mDirty = false;
}

void ConfigureData::save() const
{
    if ( mDirty )
    {
        QSettings conf( ORG_STRING, APP_STRING );
        conf.setValue( "listen/address", mServer );
        conf.setValue( "listen/port", mPort );
        conf.setValue( "package/command", mCommand );
        conf.setValue( "package/documentRoot", mDocRoot );
        mDirty = false;
    }
}

ConfigureData &ConfigureData::operator=( const ConfigureData &rhs )
{
    this->mServer = rhs.mServer;
    this->mPort = rhs.mPort;
    this->mCommand = rhs.mCommand;
    this->mDocRoot = rhs.mDocRoot;
    return *this;
}
