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

#include "perftestlinuxfbscreendriverplugin.h"
#include "perftestlinuxfbscreen.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

PerftestLinuxFbScreenDriverPlugin::PerftestLinuxFbScreenDriverPlugin( QObject *parent )
    : QScreenDriverPlugin( parent )
{}

PerftestLinuxFbScreenDriverPlugin::~PerftestLinuxFbScreenDriverPlugin()
{}

QScreen* PerftestLinuxFbScreenDriverPlugin::create(const QString& key, int displayId)
{
#ifndef QT_NO_QWS_LINUXFB
    if (key.toLower() == "perftestlinuxfb") {
        qLog(Input) << "Creating PerftestScreen()";
        return new PerftestLinuxFbScreen(displayId);
    }
#endif
    return 0;
}

QStringList PerftestLinuxFbScreenDriverPlugin::keys() const
{
    return QStringList()
#ifndef QT_NO_QWS_LINUXFB
        << "perftestlinuxfb"
#endif
    ;
}

QTOPIA_EXPORT_QT_PLUGIN(PerftestLinuxFbScreenDriverPlugin)
