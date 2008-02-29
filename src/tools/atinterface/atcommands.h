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

#ifndef ATCOMMANDS_H
#define ATCOMMANDS_H

#include "atsessionmanager.h"
#include "atoptions.h"
#include "atindicators.h"
#include "atfrontend.h"
#include "atcallmanager.h"
#include "atcustom.h"

#include <QServiceNumbers>
#include <QSlotInvoker>
#include <QMap>

#ifdef QTOPIA_CELL
class AtGsmCellCommands;
#endif
#ifdef QTOPIA_BLUETOOTH
class AtBluetoothCommands;
#endif
#ifdef ATINTERFACE_SMS
class AtSmsCommands;
#endif
class AtGsmNonCellCommands;
class AtV250Commands;
class AtManager;
class AtFrontEnd;
class QNetworkRegistration;

class AtCommands : public QObject
{
    Q_OBJECT
public:
    AtCommands( AtFrontEnd *frontEnd, AtSessionManager *manager );
    ~AtCommands();

    AtFrontEnd *frontEnd() const;
    AtSessionManager *manager() const;
    AtOptions *options() const;
    AtGsmNonCellCommands *atgnc() const;

    bool invokeCommand( const QString& cmd, const QString& params );
    void add( const QString& name, QObject *target, const char *slot );

    QAtResult::ResultCode extendedError;
    QAtResult::ResultCode result;
    QStringList cmds;
    int cmdsPosn;

public slots:
    void send( const QString& line );
    void done( QAtResult::ResultCode result = QAtResult::OK );
    void doneWithExtendedError( QAtResult::ResultCode result );
    void ignore();
    void notAllowed();

    // these AT commands are "ambiguous" - used by both cell & noncell.
    void atd( const QString& params );
    void atcfun( const QString& params );
    void atcimi( const QString& params );

private slots:
    void commands( const QStringList& cmds );
    void processNextCommand();

private:
    AtFrontEnd *atFrontEnd;
    AtSessionManager *atManager;

    AtGsmNonCellCommands *m_atgnc;
#ifdef QTOPIA_CELL
    AtGsmCellCommands *m_atgcc;
#endif
#ifdef ATINTERFACE_SMS
    AtSmsCommands *m_atsms;
#endif
#ifdef QTOPIA_BLUETOOTH
    AtBluetoothCommands *m_atbtc;
#endif
    AtV250Commands *m_atv250c;

    bool dataCallRequested;
    QMap<QString, QSlotInvoker *> invokers;

private slots:
    void stateChanged( int callID, AtCallManager::CallState state,
                       const QString& number, const QString& type );
    void deferredResult( AtCommands *handler, QAtResult::ResultCode result );
    void ring( const QString& number, const QString& type );
    void dialingOut( bool asynchronous, bool transparent, bool gprs );
    void outgoingConnected( const QString& number );
    void callWaiting( const QString& number, const QString& type );
    void noCarrier();


};

#endif // ATCOMMANDS_H
