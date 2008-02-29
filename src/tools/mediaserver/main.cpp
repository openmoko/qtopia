/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#include <qtopiaapplication.h>
#include <QDrmContentPlugin>
#include <custom-qtopia.h>

#include "sessionmanager.h"
#include "qtopiamediaprovider.h"
#include "qsoundprovider.h"
#include "domainmanagerservice.h"
#include "qaudiostatemanager.h"


#ifndef QTOPIA_NO_MEDIAPOWERCONTROL
#include "mediapowercontrol.h"
#endif

#ifdef SINGLE_EXEC
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET,mediaserver)
#define MAIN_FUNC main_mediaserver
#else
#define MAIN_FUNC main
#endif

using namespace mediaserver;



QSXE_APP_KEY
int MAIN_FUNC(int argc, char** argv)
{
    QSXE_SET_APP_KEY(argv[0])

    QtopiaApplication   app(argc, argv);

    // Set the preferred document system connection type
    QTOPIA_SET_DOCUMENT_SYSTEM_CONNECTION();

#ifndef QT_NO_SXE
    app.setSxeAuthorizerRole( QtopiaApplication::SxeAuthorizerServerAndClient );
#endif

    QAudioStateManager audiomgr;

    QtopiaMediaProvider     qmp(SessionManager::instance());
    QSoundProvider          qsp(SessionManager::instance());
    DomainManagerService    dms;

#ifndef QTOPIA_NO_MEDIAPOWERCONTROL
    MediaPowerControl       mpc;

    QObject::connect(SessionManager::instance(), SIGNAL(activeSessionCountChanged(int)),
                     &mpc, SLOT(activeSessionCount(int)));
#endif

    app.registerRunningTask("SessionManager", SessionManager::instance());

    QDrmContentPlugin::initialize();

    return app.exec();
}

