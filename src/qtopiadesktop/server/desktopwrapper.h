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
#ifndef DESKTOPWRAPPER_H
#define DESKTOPWRAPPER_H

#include <qdplugindefs.h>
#include <center.h>

#include <QObject>

class QtopiaDesktopApplication;

class DesktopWrapper : public QObject, public CenterInterface
{
public:
    DesktopWrapper( QDPlugin *plugin );
    ~DesktopWrapper();

    QDDevPlugin *currentDevice();

    const QDLinkPluginList linkPlugins();
    const QDDevPluginList devicePlugins();
    QDPlugin *getPlugin( const QString &id );
    QObject *syncObject();

private:
    QDPlugin *plugin;
    QtopiaDesktopApplication *q;
};


#endif
