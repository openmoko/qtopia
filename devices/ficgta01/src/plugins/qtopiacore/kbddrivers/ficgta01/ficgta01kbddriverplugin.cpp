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

#include "ficgta01kbddriverplugin.h"
#include "ficgta01kbdhandler.h"

#include <qtopiaglobal.h>

Ficgta01KbdDriverPlugin::Ficgta01KbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{}

Ficgta01KbdDriverPlugin::~Ficgta01KbdDriverPlugin()
{}

QWSKeyboardHandler* Ficgta01KbdDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("Ficgta01KbdDriverPlugin:create()");
    return create( driver );
}

QWSKeyboardHandler* Ficgta01KbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "ficgta01kbdhandler" ) {
        qWarning("Before call Ficgta01KbdHandler()");
        return new Ficgta01KbdHandler();
    }
    return 0;
}

QStringList Ficgta01KbdDriverPlugin::keys() const
{
    return QStringList() << "ficgta01kbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(Ficgta01KbdDriverPlugin)
