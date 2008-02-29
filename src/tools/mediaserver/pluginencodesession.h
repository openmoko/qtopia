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

#ifndef __QTOPIA_MEDIASERVER_PLUGINENCODESESSION_H
#define __QTOPIA_MEDIASERVER_PLUGINENCODESESSION_H

#include <qstring.h>

#include "mediasession.h"


class QMediaEncoder;
class QMediaDevice;


namespace qtopia_helix
{

class PluginEncodeSession : public mediaserver::MediaSession
{
    Q_OBJECT

    struct PluginEncodeSessionPrivate;

public:
    PluginEncodeSession(QMediaDevice* inputDevice,
                        QMediaEncoder* decoder,
                        QMediaDevice* outputDevice);
    ~PluginEncodeSession();

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

private:
    PluginEncodeSessionPrivate* d;
};

}

#endif  // __QTOPIA_MEDIASERVER_PLUGINENCODESESSION_H

