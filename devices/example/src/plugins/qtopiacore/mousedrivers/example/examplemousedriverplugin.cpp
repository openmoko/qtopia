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

#include "examplemousedriverplugin.h"
#include "examplemousehandler.h"

#include <qtopiaglobal.h>

#include <qtopialog.h>

ExampleMouseDriverPlugin::ExampleMouseDriverPlugin( QObject *parent )
    : QMouseDriverPlugin( parent )
{
}

ExampleMouseDriverPlugin::~ExampleMouseDriverPlugin()
{
}

QWSMouseHandler* ExampleMouseDriverPlugin::create(const QString& driver, const QString& device)
{
    if ( driver.toLower() == "examplemousehandler" ) {
        qLog(Input) << "Before call ExampleMouseHandler()";
        return new ExampleMouseHandler(device);
    }
    return 0;
}

QWSMouseHandler* ExampleMouseDriverPlugin::create(const QString& driver)
{
    if( driver.toLower() == "examplemousehandler" ) {
        qLog(Input) << "Before call ExampleMouseHandler()";
        return new ExampleMouseHandler();
    }
    return 0;
}

QStringList ExampleMouseDriverPlugin::keys() const
{
    return QStringList() << "examplemousehandler";
}

QTOPIA_EXPORT_QT_PLUGIN(ExampleMouseDriverPlugin)
