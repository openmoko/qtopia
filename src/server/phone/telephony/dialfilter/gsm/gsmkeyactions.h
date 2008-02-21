/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _GSMKEYACTIONS_H_
#define _GSMKEYACTIONS_H_

#include "qtopiaserverapplication.h"
#include "gsmkeyfilter.h"
#include "qtelephonynamespace.h"

class GsmKeyActionsPrivate;
class QDialOptions;
class QAbstractDialerScreen;

class GsmKeyActions : public QObject
{
    Q_OBJECT
public:
    explicit GsmKeyActions( QObject *parent = 0 );
    virtual ~GsmKeyActions();

    void setDialer( QAbstractDialerScreen *dialer );
    bool isGsmNumber( const QString& number );

private slots:
    void imeiRequest();
    void imeiReply( const QString& name, const QString& value );
    void changePassword( const QString& request );
    void changePin( const QString& request );
    void changePin2( const QString& request );
    void unblockPin( const QString& request );
    void unblockPin2( const QString& request );
    void modifyDial( QDialOptions& options, bool& handledAlready );
    void filterKeys( const QString& input, bool& filtered );
    void filterSelect( const QString& input, bool& filtered );
    void testKeys( const QString& input, bool& filterable );
    void callerIdRestriction
        ( GsmKeyFilter::ServiceAction action, const QStringList& args );
    void callForwarding
        ( GsmKeyFilter::ServiceAction action, const QStringList& args );
    void callBarring
        ( GsmKeyFilter::ServiceAction action, const QStringList& args );
    void callerIdPresentation
        ( GsmKeyFilter::ServiceAction action, const QStringList& args );
    void connectedIdPresentation
        ( GsmKeyFilter::ServiceAction action, const QStringList& args );
    void holdOrSwap();
    void releaseAllAcceptIncoming();
    void supplementaryServiceResult( QTelephony::Result result );

private:
    GsmKeyActionsPrivate *d;

    bool checkNewPins( const QString& title, const QStringList& pins );
    void sendServiceToNetwork
        ( GsmKeyFilter::ServiceAction action, const QStringList& args,
          const QString& title = QString() );
};

#endif // _GSMKEYACTIONS_H_
