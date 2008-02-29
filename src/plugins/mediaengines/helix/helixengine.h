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

#ifndef __QTOPIA_HELIXENGINE_H
#define __QTOPIA_HELIXENGINE_H

#include <qmediaengine.h>
#include <qmediasessionbuilder.h>

class QTimerEvent;
class QMediaEngineInformation;
class QMediaSessionRequest;
class QMediaServerSession;
class IHXClientEngine;


namespace qtopia_helix
{

class HelixEnginePrivate;

class HelixEngine :
    public QMediaEngine,
    public QMediaSessionBuilder
{
    Q_OBJECT

public:
    HelixEngine();
    ~HelixEngine();

    // QMediaEngine
    void initialize();

    void start();
    void stop();

    void suspend();
    void resume();

    QMediaEngineInformation const* engineInformation();

    // QMediaSessionBuilder
    QString type() const;
    QMediaSessionBuilder::Attributes const& attributes() const;

    QMediaServerSession* createSession(QMediaSessionRequest sessionRequest);
    void destroySession(QMediaServerSession* serverSession);

private:
    void timerEvent(QTimerEvent* timerEvent);

    HelixEnginePrivate* d;
};

}   // ns qtopia_helix


#endif  //__QTOPIA_HELIXENGINE_H


