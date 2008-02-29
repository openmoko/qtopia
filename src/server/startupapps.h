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

#ifndef _STARTUPAPPS_H_
#define _STARTUPAPPS_H_

#include <QObject>
#include "applicationlauncher.h"

class StartupApplications : public QObject
{
Q_OBJECT
public:
    StartupApplications(QObject *parent = 0);

signals:
    void preloadCrashed(const QString &);
};
QTOPIA_TASK_INTERFACE(StartupApplications);

#endif // _STARTUPAPPS_H_

