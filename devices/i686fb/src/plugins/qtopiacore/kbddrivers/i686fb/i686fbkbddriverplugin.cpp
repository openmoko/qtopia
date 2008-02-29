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

#include "i686fbkbddriverplugin.h"
#include "i686fbkbdhandler.h"

#include <qtopiaglobal.h>

I686fbKbdDriverPlugin::I686fbKbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{}

I686fbKbdDriverPlugin::~I686fbKbdDriverPlugin()
{}

QWSKeyboardHandler* I686fbKbdDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("I686fbKbdDriverPlugin:create()");
    return create( driver );
}

QWSKeyboardHandler* I686fbKbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "i686fbkbdhandler" ) {
        qWarning("Before call I686fbKbdHandler()");
        return new I686fbKbdHandler();
    }
    return 0;
}

QStringList I686fbKbdDriverPlugin::keys() const
{
    return QStringList() << "i686fbkbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(I686fbKbdDriverPlugin)
