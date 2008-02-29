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

#ifndef __QTOPIA_MEDIA_HELIXSESSION_H
#define __QTOPIA_MEDIA_HELIXSESSION_H

#include <qstring.h>
#include <quuid.h>

#include <qtopiamedia/observer.h>

#include "mediasession.h"

class IHXClientEngine;

namespace qtopia_helix
{

class HelixSessionPrivate;

class HelixSession :
    public mediaserver::MediaSession,
    public Observer
{
    Q_OBJECT

public:
    HelixSession(IHXClientEngine* engine, QUuid const& id, QString const& url);
    ~HelixSession();

    void start();
    void pause();
    void stop();

    void suspend();
    void resume();

    void seek(quint32 ms);
    quint32 length();

    void setVolume(int volume);
    int volume() const;

    void setMuted(bool mute);
    bool isMuted() const;

    QString errorString();

    void setDomain(QString const& domain);
    QString domain() const;

    QStringList interfaces();

    void update(Subject* subject);

private slots:
    void delaySeek();

private:
    void startupPlayer();
    void shutdownPlayer();

    HelixSessionPrivate*    d;
};

}


#endif  //__QTOPIA_MEDIA_HELIXSESSION_H

