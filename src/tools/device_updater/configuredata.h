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

#ifndef CONFIGUREDATA_H
#define CONFIGUREDATA_H

#include <QString>

class QSettings;

class ConfigureData
{
public:
    ConfigureData();
    ConfigureData( const ConfigureData & );
    ~ConfigureData();
    QString server() const  { return mServer; }
    quint16 port() const    { return mPort; }
    QString command() const { return mCommand; }
    QString docRoot() const { return mDocRoot; }
    void setServer( QString s )  { mDirty = true; mServer = s; }
    void setPort( quint16 p )    { mDirty = true; mPort = p; }
    void setCommand( QString c ) { mDirty = true; mCommand = c; }
    void setDocRoot( QString d ) { mDirty = true; mDocRoot = d; }
    ConfigureData &operator=( const ConfigureData &rhs );
    void load();
    void save() const;
private:
    QString mServer;
    quint16 mPort;
    QString mCommand;
    QString mDocRoot;
    mutable bool mDirty;
};

#endif
