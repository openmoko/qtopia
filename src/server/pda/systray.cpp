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

#include <qtopiaapplication.h>
#include <qpluginmanager.h>
#include <qtopialog.h>

#include <QDir>
#include <QLayout>
#include <QMessageBox>
#include <QSettings>
#include <QTranslator>

#include "systray.h"

#include <stdlib.h>

#ifdef QT_NO_COMPONENTS
#include "../plugins/applets/clockapplet/clockappletimpl.h"
#endif

SysTray::SysTray( QWidget *parent ) : QFrame( parent ), trayLayout(0), loader(0)
{
    loadApplets();
}

SysTray::~SysTray()
{
    clearApplets();
}

static int compareAppletPositions(const void *a, const void *b)
{
    const TaskbarApplet* aa = *(const TaskbarApplet**)a;
    const TaskbarApplet* ab = *(const TaskbarApplet**)b;
    int d = ab->iface->position() - aa->iface->position();
    if ( d ) return d;
    return QString::compare(ab->name,aa->name);
}

void SysTray::loadApplets()
{
    hide();
    clearApplets();
    addApplets();
}

void SysTray::clearApplets()
{
#ifndef QT_NO_COMPONENTS
    if ( loader ) {
        QList<TaskbarApplet*>::Iterator mit;
        for ( mit = appletList.begin(); mit != appletList.end(); ++mit ) {
            delete (*mit)->instance;
            delete (*mit);
        }
    }
#endif
    appletList.clear();
    if ( trayLayout )
        delete trayLayout;
    trayLayout = new QHBoxLayout(this);
    trayLayout->setMargin(0);
    trayLayout->setSpacing(1);
    delete loader;
    loader = 0;
}

void SysTray::addApplets()
{
    hide();
    delete loader;
    loader = new QPluginManager( "applets" );
#ifndef QT_NO_COMPONENTS
    QStringList faulty;

    QStringList list = loader->list();
    TaskbarApplet **applets = new TaskbarApplet* [list.count()];
    QStringList::Iterator it;
    int napplets=0;
    for ( it = list.begin(); it != list.end(); ++it ) {
        QObject *instance = loader->instance(*it);
        QTaskbarAppletPlugin *iface = 0;
        iface = qobject_cast<QTaskbarAppletPlugin*>(instance);
        if ( iface ) {
            TaskbarApplet *applet = new TaskbarApplet;
            applets[napplets++] = applet;
            applet->instance = instance;
            applet->iface = iface;
            applet->name = *it;
            qLog(UI) << "Added applet: " <<  applet->name.toAscii();
        } else {
#ifndef Q_OS_WIN32
            // Same as Taskbar settings uses
            QString name = (*it).mid(3);
            int sep = name.indexOf( ".so" );
#else
            QString name = (*it);
            int sep = name.indexOf( ".dll" );
#endif
            if ( sep > 0 )
                name.truncate( sep );
            sep = name.indexOf( "applet" );
            if ( sep == (int)name.length() - 6 )
                name.truncate( sep );
            name[0] = name[0].toUpper();
            faulty += name;
            // we don't do anything with faulty anymore -
            // maybe we should.
        }
    }

    qsort(applets,napplets,sizeof(applets[0]),compareAppletPositions);
    while (napplets--) {
        TaskbarApplet *applet = applets[napplets];
        applet->applet = applet->iface->applet( this );
        applet->applet->setBackgroundRole( QPalette::Button );
        if ( applet->applet->maximumSize().width() <= 1 )
            applet->applet->hide();
        appletList.append(applet);
        trayLayout->addWidget(applet->applet);
    }
    delete [] applets;
#else
    TaskbarApplet * const applet = new TaskbarApplet();
    applet->iface = new ClockAppletImpl();
    applet->applet = applet->iface->applet( this );
    appletList.append( applet );
#endif
    show();
}

