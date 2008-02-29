/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef NETWORKINTERFACE_H
#define NETWORKINTERFACE_H

#include <qnamespace.h>
#include <qstring.h>
#include <qvariant.h>
#include <qtopia/qcom.h>

#ifndef QT_NO_COMPONENT
// {09794290-69d5-4c42-8a53-230cf2e53c50}
// {09794290-69d5-4c42-8a53-230cf2e53c51}
# define IID_Network QUuid( 0x09794290, 0x69d5, 0x4c42, 0x8a, 0x53, 0x23, 0x0c, 0xf2, 0xe5, 0x3c, 0x50)
# define IID_Network2 QUuid( 0x09794290, 0x69d5, 0x4c42, 0x8a, 0x53, 0x23, 0x0c, 0xf2, 0xe5, 0x3c, 0x51)
#endif

class QWidget;
class Config;

struct QTOPIA_EXPORT NetworkInterface : public QUnknownInterface
{
    virtual bool doProperties( QWidget *parent, Config& ) = 0;
    virtual bool create( Config& ) = 0;
    virtual bool remove( Config& ) = 0;
    virtual QString device( Config& ) const;
    virtual QString cardType( Config& ) const;
    virtual bool isActive( Config& ) const;
    virtual bool isAvailable( Config& ) const;
    virtual bool start( Config& ) = 0;
    virtual bool start( Config&, const QString& password );
    virtual bool stop( Config& ) = 0;
    virtual bool needPassword( Config& ) const;
    virtual QWidget* addStateWidget( QWidget* parent, Config& ) const;
};

class NetworkInterfaceProperties : public QMap<QString, QVariant>
{
public:
    NetworkInterfaceProperties() {}
    NetworkInterfaceProperties( const NetworkInterfaceProperties& list )
	: QMap<QString, QVariant> ( list ) {}
    ~NetworkInterfaceProperties() {}

    void debugDump() const;
};

struct QTOPIA_EXPORT NetworkInterface2 : public NetworkInterface
{
    virtual bool setProperties( Config&, const NetworkInterfaceProperties& ) = 0;
};

#endif

