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

#include "zylonitemousedriverplugin.h"
#include "zylonitemousehandler.h"

#include <qtopiaglobal.h>

ZyloniteMouseDriverPlugin::ZyloniteMouseDriverPlugin( QObject *parent )
    : QMouseDriverPlugin( parent )
{}

ZyloniteMouseDriverPlugin::~ZyloniteMouseDriverPlugin()
{}

QWSMouseHandler* ZyloniteMouseDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("ZyloniteMouseDriverPlugin:create()");
    return create( driver );
}

QWSMouseHandler* ZyloniteMouseDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "zylonitemousehandler" ) {
        qWarning("Before call ZyloniteMouseHandler()");
        return new ZyloniteMouseHandler();
    }
    return 0;
}

QStringList ZyloniteMouseDriverPlugin::keys() const
{
    return QStringList() << "zylonitemousehandler";
}

QTOPIA_EXPORT_QT_PLUGIN(ZyloniteMouseDriverPlugin)
