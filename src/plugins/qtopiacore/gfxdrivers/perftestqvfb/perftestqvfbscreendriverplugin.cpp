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

#include "perftestqvfbscreendriverplugin.h"
#include "perftestqvfbscreen.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

PerftestQVFbScreenDriverPlugin::PerftestQVFbScreenDriverPlugin( QObject *parent )
    : QScreenDriverPlugin( parent )
{}

PerftestQVFbScreenDriverPlugin::~PerftestQVFbScreenDriverPlugin()
{}

QScreen* PerftestQVFbScreenDriverPlugin::create(const QString& key, int displayId)
{
#ifndef QT_NO_QWS_QVFB
    if (key.toLower() == "perftestqvfb") {
        qLog(Input) << "Creating PerftestScreen()";
        return new PerftestQVFbScreen(displayId);
    }
#endif
    return 0;
}

QStringList PerftestQVFbScreenDriverPlugin::keys() const
{
    return QStringList()
#ifndef QT_NO_QWS_QVFB
        << "perftestqvfb"
#endif
    ;
}

QTOPIA_EXPORT_QT_PLUGIN(PerftestQVFbScreenDriverPlugin)
