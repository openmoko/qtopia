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

#include "nokiakbddriverplugin.h"
#include "nokiakbdhandler.h"

#include <qtopiaglobal.h>

NokiaKbdDriverPlugin::NokiaKbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{
}

NokiaKbdDriverPlugin::~NokiaKbdDriverPlugin()
{
}

QWSKeyboardHandler* NokiaKbdDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("NokiaKbdDriverPlugin:create()");
    return create( driver );
}

QWSKeyboardHandler* NokiaKbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "nokiakbdhandler" ) {
        qWarning("Before call NokiaKbdHandler()");
        return new NokiaKbdHandler();
    }
    return 0;
}

QStringList NokiaKbdDriverPlugin::keys() const
{
    return QStringList() << "nokiakbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(NokiaKbdDriverPlugin)
