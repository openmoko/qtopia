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

#include "bscidrmagentplugin.h"
#include "bscidrmcontentplugin.h"
#include "bscidrmagentservice.h"
#include "bscirightsmanager.h"
#include "bsciprompts.h"
#include <qtopiaapplication.h>
#include <QtDebug>

BSciDrmAgentPlugin::BSciDrmAgentPlugin()
{
#ifndef QT_NO_TRANSLATION
    QStringList trans;
    trans << "libqtopiaomadrm";
    QtopiaApplication::loadTranslations(trans);
#endif

    if( !BSciDrm::context )
    {
        QString appId = QLatin1String( "QtopiaDrm-" ) + qApp->applicationName();

        BSciDrm::initialiseAgent( appId, BSciPrompts::instance()->callbacks() );
    }
    else
        qWarning() << "Multiple BSciDrmAgentPlugin instances";
}

BSciDrmAgentPlugin::~BSciDrmAgentPlugin()
{
    BSciDrm::releaseAgent();
}

QDrmContentPlugin *BSciDrmAgentPlugin::createDrmContentPlugin()
{
    return new BSciDrmContentPlugin();
}

void BSciDrmAgentPlugin::createService( QObject *parent )
{
    new BSciDrmAgentService( parent );
}

QList< QWidget * > BSciDrmAgentPlugin::createManagerWidgets( QWidget *parent )
{
    QList< QWidget * > widgets;

    BSciRightsView *view = new BSciRightsView( parent );

    view->setModel( new BSciRightsModel( view ) );

    view->setWindowTitle( tr( "Licenses" ) );

    widgets.append( view );

    BSciSettings *settings = new BSciSettings( parent );

    settings->setWindowTitle( tr( "Settings" ) );

    widgets.append( settings );

    return widgets;
}

QTOPIA_EXPORT_PLUGIN(BSciDrmAgentPlugin);

