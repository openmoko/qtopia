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

#include "zylonitekbddriverplugin.h"
#include "zylonitekbdhandler.h"

#include <qtopiaglobal.h>

ZyloniteKbdDriverPlugin::ZyloniteKbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{}

ZyloniteKbdDriverPlugin::~ZyloniteKbdDriverPlugin()
{}

QWSKeyboardHandler* ZyloniteKbdDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("ZyloniteKbdDriverPlugin:create()");
    return create( driver );
}

QWSKeyboardHandler* ZyloniteKbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "zylonitekbdhandler" ) {
        qWarning("Before call ZyloniteKbdHandler()");
        return new ZyloniteKbdHandler();
    }
    return 0;
}

QStringList ZyloniteKbdDriverPlugin::keys() const
{
    return QStringList() << "zylonitekbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(ZyloniteKbdDriverPlugin)
