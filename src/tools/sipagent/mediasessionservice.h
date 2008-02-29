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

#ifndef MEDIASESSIONSERVICE_H
#define MEDIASESSIONSERVICE_H

#include <qtopiaabstractservice.h>

class MediaSessionService : public QtopiaAbstractService
{
    Q_OBJECT
protected:
    MediaSessionService( QObject *parent = 0 );

public:
    ~MediaSessionService();

public slots:
    virtual void negotiate( const QString& id, const QString& remotesdp ) = 0;
    virtual void start( const QString& id, const QString& localsdp,
                        const QString& remotesdp ) = 0;
    virtual void stop( const QString& id ) = 0;
    virtual void suspend( const QString& id ) = 0;
    virtual void dtmf( const QString& id, const QString& tones ) = 0;

protected:
    void negotiateResult( const QString& id, const QString& localsdp );
    void startFailed( const QString& id );
    void renegotiate( const QString& id );
};

#endif /* MEDIASESSIONSERVICE_H */
