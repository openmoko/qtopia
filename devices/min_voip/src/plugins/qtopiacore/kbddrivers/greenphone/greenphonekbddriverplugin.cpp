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

#include "greenphonekbddriverplugin.h"
#include "greenphonekbdhandler.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

GreenphoneKbdDriverPlugin::GreenphoneKbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{}

GreenphoneKbdDriverPlugin::~GreenphoneKbdDriverPlugin()
{}

QWSKeyboardHandler* GreenphoneKbdDriverPlugin::create(const QString& driver, const QString&)
{
    qLog(Input) << "GreenphoneKbdDriverPlugin:create()";
    return create( driver );
}

QWSKeyboardHandler* GreenphoneKbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "greenphonekbdhandler" ) {
        qLog(Input) << "Before call GreenphoneKbdHandler()";
        return new GreenphoneKbdHandler();
    }
    return 0;
}

QStringList GreenphoneKbdDriverPlugin::keys() const
{
    return QStringList() << "greenphonekbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(GreenphoneKbdDriverPlugin)
