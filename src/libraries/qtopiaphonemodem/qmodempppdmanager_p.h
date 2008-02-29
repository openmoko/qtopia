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

#ifndef QMODEMPPPDMANAGER_P_H
#define QMODEMPPPDMANAGER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qdialoptions.h>
#include <qphonecall.h>
#include <qphonecallmanager.h>
#include <qbytearray.h>
#include <qprocess.h>

class QSerialIODeviceMultiplexer;
class QProcess;
class QtopiaChannel;

class QModemPPPdManager : public QObject
{
    Q_OBJECT
public:
    QModemPPPdManager( QSerialIODeviceMultiplexer *mux, QObject *parent );
    ~QModemPPPdManager();

    bool isActive() const;
    bool dataCallsPossible() const;
    bool start( const QDialOptions& options );
    void stop();

private:
    void dataCallStop();
    void raiseDtr();
    void runChatFile( const QString& filename, const char *slot );
    void changeState( int value );

private slots:
    void pppdListen( const QString& msg, const QByteArray& data );
    void raiseDtrAndHangup();
    void connected( bool ok );
    void disconnected();
    void pppdStateChanged( QProcess::ProcessState state );

signals:
    void dataCallActive();
    void dataCallInactive();
    void dataStateUpdate( QPhoneCall::DataState state );

private:
    QSerialIODeviceMultiplexer *mux;
    QProcess *process;
    QDialOptions options;
    QtopiaChannel *pppdChannel;
    bool active;
    bool terminated;
};

#endif // QMODEMPPPDMANAGER_P_H
