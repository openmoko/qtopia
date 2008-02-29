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

#include "pxa27xmousedriverplugin.h"
#include "pxa27xmousehandler.h"

#include <qtopiaglobal.h>

Pxa27xMouseDriverPlugin::Pxa27xMouseDriverPlugin( QObject *parent )
    : QMouseDriverPlugin( parent )
{}

Pxa27xMouseDriverPlugin::~Pxa27xMouseDriverPlugin()
{}

QWSMouseHandler* Pxa27xMouseDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("Pxa27xMouseDriverPlugin:create()");
    return create( driver );
}

QWSMouseHandler* Pxa27xMouseDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "pxa27xmousehandler" ) {
        qWarning("Before call pxa27xMouseHandler()");
        return new Pxa27xMouseHandler();
    }
    return 0;
}

QStringList Pxa27xMouseDriverPlugin::keys() const
{
    return QStringList() << "pxa27xmousehandler" << "pxa27x_ts";
}

QTOPIA_EXPORT_QT_PLUGIN(Pxa27xMouseDriverPlugin)
