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

#include "c3200kbddriverplugin.h"
#include "c3200kbdhandler.h"

#include <qtopiaglobal.h>

C3200KbdDriverPlugin::C3200KbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{}

C3200KbdDriverPlugin::~C3200KbdDriverPlugin()
{}

QWSKeyboardHandler* C3200KbdDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("C3200KbdDriverPlugin:create()");
    return create( driver );
}

QWSKeyboardHandler* C3200KbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "c3200kbdhandler" ) {
        qWarning("Before call C3200KbdHandler()");
        return new C3200KbdHandler();
    }
    return 0;
}

QStringList C3200KbdDriverPlugin::keys() const
{
    return QStringList() << "c3200kbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(C3200KbdDriverPlugin)
