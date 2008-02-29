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

#ifndef SIPAGENTMEDIA_H
#define SIPAGENTMEDIA_H

#include "mediasessionservice.h"
#include <qmap.h>
#include <QNetworkInterface>

class SipAgentMediaSession;
class QNetworkState;

class SipAgentMedia : public MediaSessionService
{
    Q_OBJECT
public:
    SipAgentMedia( QObject *parent = 0 );
    ~SipAgentMedia();

public slots:
    void negotiate( const QString& id, const QString& remotesdp );
    void start( const QString& id, const QString& localsdp,
                const QString& remotesdp );
    void stop( const QString& id );
    void suspend( const QString& id );
    void dtmf( const QString& id, const QString& tones );

    static bool supportsCodecs( const QStringList& codecs );

private slots:
    void networkChanged( QString iface, const QNetworkInterface& newAddress );

private:
    QMap<QString, SipAgentMediaSession *> sessions;
    SipAgentMediaSession *currentSession;
    QNetworkState *networkState;
    QHostAddress localAddress;

    static QStringList configuredCodecs();
    static QStringList commonCodecs( const QStringList& ourCodecs,
                                     const QStringList& theirCodecs );
};

#endif /* SIPAGENTMEDIA_H */
