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

#ifndef __QTOPIA_MEDIASERVER_MEDIASESSION_H
#define __QTOPIA_MEDIASERVER_MEDIASESSION_H

#include <QObject>
#include <QStringList>

#include <qtopiaglobal.h>

#include <media.h>


class QTOPIAMEDIA_EXPORT QMediaServerSession : public QObject
{
    Q_OBJECT

public:
    virtual ~QMediaServerSession();

    virtual void start() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;

    virtual void suspend() = 0;
    virtual void resume() = 0;

    virtual void seek(quint32 ms) = 0;
    virtual quint32 length() = 0;

    virtual void setVolume(int volume) = 0;
    virtual int volume() const = 0;

    virtual void setMuted(bool mute) = 0;
    virtual bool isMuted() const = 0;

    virtual QtopiaMedia::State playerState() const = 0;

    virtual QString errorString() = 0;

    virtual void setDomain(QString const& domain) = 0;
    virtual QString domain() const = 0;

    virtual QStringList interfaces() = 0;

    virtual QString id() const = 0;
    virtual QString reportData() const = 0;

signals:
    void playerStateChanged(QtopiaMedia::State state);
    void positionChanged(quint32 ms);
    void lengthChanged(quint32 ms);
    void volumeChanged(int volume);
    void volumeMuted(bool muted);
    void interfaceAvailable(const QString& name);
    void interfaceUnavailable(const QString& name);
};


#endif  // __QTOPIA_MEDIASERVER_MEDIASESSION_H
