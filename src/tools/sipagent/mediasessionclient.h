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

#ifndef MEDIASESSIONCLIENT_H
#define MEDIASESSIONCLIENT_H

#include <qobject.h>
#include <qstring.h>
#include <qbytearray.h>

class MediaSessionClientPrivate;

class MediaSessionClient : public QObject
{
    Q_OBJECT
public:
    MediaSessionClient( QObject *parent = 0 );
    ~MediaSessionClient();

    bool remoteOnHold() const;
    bool localOnHold() const;
    bool onHold() const { return remoteOnHold() || localOnHold(); }

public slots:
    void setupLocal();
    void setupRemote( const QString& sdp );
    void changeRemoteParameters( const QString& sdp );
    void setLocalOnHold( bool flag );
    void end();
    void dtmf( const QString& tones );

signals:
    void sendLocalParameters( const QString& sdp );
    void onHoldChanged();
    void remoteOnHoldChanged();
    void sessionFailed();

private slots:
    void received( const QString& msg, const QByteArray& data );

private:
    MediaSessionClientPrivate *d;

    void startMedia();
    void stopMedia();
    void suspendMedia();
    void remoteHold();
    void remoteSet( const QString& sdp );
    void localHold();
    void localUnhold();
};

#endif /* MEDIASESSIONCLIENT_H */
