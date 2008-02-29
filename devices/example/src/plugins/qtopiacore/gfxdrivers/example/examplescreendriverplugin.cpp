/****************************************************************************
**
** Copyright (C) 2007-2007 TROLLTECH ASA. All rights reserved.
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

#include "examplescreendriverplugin.h"
#include "examplescreen.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

ExampleScreenDriverPlugin::ExampleScreenDriverPlugin( QObject *parent )
: QScreenDriverPlugin( parent )
{
}

ExampleScreenDriverPlugin::~ExampleScreenDriverPlugin()
{
}

QScreen* ExampleScreenDriverPlugin::create(const QString& key, int displayId)
{
    if (key.toLower() == "examplescreen") 
        return new ExampleScreen(displayId);
    
    return 0;
}

QStringList ExampleScreenDriverPlugin::keys() const
{
    return QStringList() << "ExampleScreen";
}

QTOPIA_EXPORT_QT_PLUGIN(ExampleScreenDriverPlugin)

