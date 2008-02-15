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

#include "perftestlinuxfbscreen.h"

#ifndef QT_NO_QWS_LINUXFB

#include <QScreenDriverFactory>
#include <QWSServer>
#include <QPerformanceLog>
#include <qtopialog.h>
#include <QDebug>


PerftestLinuxFbScreen::PerftestLinuxFbScreen(int display_id) : QLinuxFbScreen(display_id)
{
    QPerformanceLog() << "Created Perftest LINUXFB screen driver";
}

PerftestLinuxFbScreen::~PerftestLinuxFbScreen()
{
}

bool PerftestLinuxFbScreen::connect(const QString &displaySpec)
{
    QString spec(displaySpec);
    if (spec.startsWith("perftestlinuxfb:"))
        spec = spec.mid(16);

    QPerformanceLog() << "Attempting to connect with spec" << ((spec.isEmpty()) ? "(none)" : spec);

    return QLinuxFbScreen::connect(spec);
}

void PerftestLinuxFbScreen::exposeRegion(QRegion region, int windowIndex)
{
    if (QPerformanceLog::enabled()) {
        QWSWindow *changed = 0;
        if (windowIndex >= 0 && QWSServer::instance() && QWSServer::instance()->clientWindows().count() > windowIndex)
            changed = QWSServer::instance()->clientWindows().at(windowIndex);
        if(changed && !changed->client()->identity().isEmpty()) {
            QRect r = region.boundingRect();
            QPerformanceLog()
                << changed->client()->identity()
                << ": expose_region"
                << QString("QRect(%1,%2 %3x%4)").arg(r.left()).arg(r.top()).arg(r.width()).arg(r.height());
        }
    }

    return QLinuxFbScreen::exposeRegion(region, windowIndex);
}

#endif
