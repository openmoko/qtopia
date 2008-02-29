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

#include "pxa27xkbddriverplugin.h"
#include "pxa27xkbdhandler.h"

#include <qtopiaglobal.h>

Pxa27xKbdDriverPlugin::Pxa27xKbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{}

Pxa27xKbdDriverPlugin::~Pxa27xKbdDriverPlugin()
{}

QWSKeyboardHandler* Pxa27xKbdDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("Pxa27xKbdDriverPlugin:create()");
    return create( driver );
}

QWSKeyboardHandler* Pxa27xKbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "pxa27xkbdhandler" ) {
        qWarning("Before call Pxa27xKbdHandler()");
        return new Pxa27xKbdHandler();
    }
    return 0;
}

QStringList Pxa27xKbdDriverPlugin::keys() const
{
    return QStringList() << "pxa27xkbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(Pxa27xKbdDriverPlugin)
