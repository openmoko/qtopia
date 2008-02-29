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

#ifndef __QTOPIA_SERVER_MEDIASERVER_H
#define __QTOPIA_SERVER_MEDIASERVER_H

#include "qtopiaserverapplication.h"
#include "applicationlauncher.h"


class MediaServerControlTaskPrivate;

class MediaServerControlTask : public SystemShutdownHandler
{
    Q_OBJECT

public:
    MediaServerControlTask();

    bool systemRestart();
    bool systemShutdown();

protected:
    void timerEvent(QTimerEvent *e);

private slots:
    void soundServerExited();
    void killtimeout();
    void applicationTerminated(QString const& name,
                               ApplicationTypeLauncher::TerminationReason reason,
                               bool filtered = false);

private:
    bool doShutdown();

    MediaServerControlTaskPrivate*  d;
};

#endif  // __QTOPIA_SERVER_MEDIASERVER_H
