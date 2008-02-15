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

#include "perftestqvfbscreen.h"

#ifndef QT_NO_QWS_QVFB

#include <QWSServer>
#include <QPerformanceLog>
#include <qtopialog.h>


PerftestQVFbScreen::PerftestQVFbScreen(int display_id) : QVFbScreen(display_id)
{
    QPerformanceLog() << "Created Perftest QVFB screen driver";
}

PerftestQVFbScreen::~PerftestQVFbScreen()
{
}

bool PerftestQVFbScreen::connect(const QString &displaySpec)
{
    QString spec(displaySpec);
    if (spec.startsWith("perftestqvfb:"))
        spec = spec.mid(13);

    QPerformanceLog() << "Attempting to connect with spec" << ((spec.isEmpty()) ? "(none)" : spec);

    return QVFbScreen::connect(spec);
}

void PerftestQVFbScreen::exposeRegion(QRegion region, int windowIndex)
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

    QVFbScreen::exposeRegion(region, windowIndex);
}

#endif
