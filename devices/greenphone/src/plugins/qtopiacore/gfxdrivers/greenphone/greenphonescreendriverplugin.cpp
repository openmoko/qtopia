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

#include "greenphonescreendriverplugin.h"
#include "greenphonescreen.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

GreenphoneScreenDriverPlugin::GreenphoneScreenDriverPlugin( QObject *parent )
: QScreenDriverPlugin( parent )
{
}

GreenphoneScreenDriverPlugin::~GreenphoneScreenDriverPlugin()
{
}

QScreen* GreenphoneScreenDriverPlugin::create(const QString& key, int displayId)
{
    if (key.toLower() == "greenphone") 
        return new GreenphoneScreen(displayId);
    
    return 0;
}

QStringList GreenphoneScreenDriverPlugin::keys() const
{
    return QStringList() << "greenphone";
}

QTOPIA_EXPORT_QT_PLUGIN(GreenphoneScreenDriverPlugin)

