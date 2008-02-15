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

#include "c3200mousedriverplugin.h"
#include "c3200mousehandler.h"

#include <qtopiaglobal.h>

C3200MouseDriverPlugin::C3200MouseDriverPlugin( QObject *parent )
    : QMouseDriverPlugin( parent )
{}

C3200MouseDriverPlugin::~C3200MouseDriverPlugin()
{}

QWSMouseHandler* C3200MouseDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("C3200MouseDriverPlugin:create()");
    return create( driver );
}

QWSMouseHandler* C3200MouseDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "c3200mousehandler" ) {
        qWarning("Before call C3200MouseHandler()");
        return new C3200MouseHandler();
    }
    return 0;
}

QStringList C3200MouseDriverPlugin::keys() const
{
    return QStringList() << "c3200mousehandler";
}

QTOPIA_EXPORT_QT_PLUGIN(C3200MouseDriverPlugin)
