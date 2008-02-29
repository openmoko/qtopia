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
#ifndef CENTER_H
#define CENTER_H

#include <qdplugindefs.h>
#include <qdglobal.h>

// This is the interface presented to the plugins
class QD_EXPORT CenterInterface
{
public:
#ifdef Q_OS_WIN32
    // This is needed for DLL wierdness
    CenterInterface();
#endif
#ifdef Q_OS_UNIX
    virtual ~CenterInterface();
#endif

    virtual QDDevPlugin *currentDevice() = 0;
    virtual const QDLinkPluginList linkPlugins() = 0;
    virtual const QDDevPluginList devicePlugins() = 0;
    virtual QDPlugin *getPlugin( const QString &id ) = 0;
    virtual QObject *syncObject() = 0;
};

#endif
