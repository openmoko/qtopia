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

#include "examplekbddriverplugin.h"
#include "examplekbdhandler.h"

#include <qtopiaglobal.h>

#include <qtopialog.h>

ExampleKbdDriverPlugin::ExampleKbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{
}

ExampleKbdDriverPlugin::~ExampleKbdDriverPlugin()
{
}

QWSKeyboardHandler* ExampleKbdDriverPlugin::create(const QString& driver,
                                                   const QString& device)
{
    if (driver.toLower() == "examplekbdhandler") {
        qLog(Input) << "Before call ExampleKbdHandler()";
        return new ExampleKbdHandler(device);
    }
    return 0;
}

QWSKeyboardHandler* ExampleKbdDriverPlugin::create(const QString& driver)
{
    if (driver.toLower() == "examplekbdhandler") {
        qLog(Input) << "Before call ExampleKbdHandler()";
        return new ExampleKbdHandler();
    }
    return 0;
}

QStringList ExampleKbdDriverPlugin::keys() const
{
    return QStringList() << "examplekbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(ExampleKbdDriverPlugin)
