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

#ifndef __QTOPIA_SERVER_MEDIASERVER_H
#define __QTOPIA_SERVER_MEDIASERVER_H

#include <QObject>

#include "qtopiaserverapplication.h"

class QProcess;

class MediaServerControlTask : public SystemShutdownHandler
{
Q_OBJECT
public:
    MediaServerControlTask();

    virtual bool systemRestart();
    virtual bool systemShutdown();

protected:
    virtual void timerEvent(QTimerEvent *e);

private slots:
    void soundServerExited();
    void soundServerReadyStdout();
    void soundServerReadyStderr();
    void killtimeout();

private:
    bool doShutdown();

    QProcess *m_soundserver;
    int m_qssTimerId;
    bool m_shutdown;
    bool m_shutdownCompleted;
};

#endif  // __QTOPIA_SERVER_MEDIASERVER_H
