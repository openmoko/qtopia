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

#include "omap730kbddriverplugin.h"
#include "omap730kbdhandler.h"

#include <qtopiaglobal.h>

Omap730KbdDriverPlugin::Omap730KbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{}

Omap730KbdDriverPlugin::~Omap730KbdDriverPlugin()
{}

QWSKeyboardHandler* Omap730KbdDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("Omap730KbdDriverPlugin:create()");
    return create( driver );
}

QWSKeyboardHandler* Omap730KbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "omap730kbdhandler" ) {
        qWarning("Before call Omap730KbdHandler()");
        return new Omap730KbdHandler();
    }
    return 0;
}

QStringList Omap730KbdDriverPlugin::keys() const
{
    return QStringList() << "omap730kbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(Omap730KbdDriverPlugin)
