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

#include "greenphonemousedriverplugin.h"
#include "greenphonemousehandler.h"

#include <qtopiaglobal.h>

GreenphoneMouseDriverPlugin::GreenphoneMouseDriverPlugin( QObject *parent )
    : QMouseDriverPlugin( parent )
{}

GreenphoneMouseDriverPlugin::~GreenphoneMouseDriverPlugin()
{}

QWSMouseHandler* GreenphoneMouseDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("GreenphoneMouseDriverPlugin:create()");
    return create( driver );
}

QWSMouseHandler* GreenphoneMouseDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "greenphonemousehandler" ) 
      return new GreenphoneMouseHandler();
    return 0;
}

QStringList GreenphoneMouseDriverPlugin::keys() const
{
    return QStringList() << "greenphonemousehandler";
}

QTOPIA_EXPORT_QT_PLUGIN(GreenphoneMouseDriverPlugin)
