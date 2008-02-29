/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "mediaimpl.h"
#include "medialauncher.h"

#include <qtopiaapplication.h>
#include <qtopialog.h>

#include <QDebug>
#include <QSettings>

ServerInterface *MediaImpl::create()
{
    QSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat); // No tr
    if (qLogEnabled(UI)) {
        qLog(UI) << "MediaImpl::create()" << Qtopia::defaultButtonsFile();
        foreach (QString k, cfg.allKeys()) {
            qLog(UI) << k << "=" << cfg.value(k);
        }
    }

    MediaLauncher *rv = new MediaLauncher( cfg, 0, Qt::FramelessWindowHint );
    return rv;
}
