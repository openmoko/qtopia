/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <qtopiacomm/private/qbluetoothpairingagent_p.h>

#include <qbluetoothlocaldevice.h>
#include <qwaitwidget.h>
#include <qtopialog.h>

#include <QMessageBox>
#include <QTimer>
#include <QtopiaApplication>

/*!
   \internal
   \class QBluetoothPairingAgent
   Convenience class for handling pairing and unpairing.

   This fills up the screen with a QWaitWidget when starting the pairing or
   unpairing process, allowing the user to cancel the process from the
   wait-widget screen. Also provides convenient way to run a pairing/
   unpairing synchronously, as if calling exec() on a dialog.
 */

QBluetoothPairingAgent::QBluetoothPairingAgent( Operation op, const QBluetoothAddress &addr, const QBluetoothLocalDevice &local, QObject *parent )
    : QObject( parent ),
      m_local( new QBluetoothLocalDevice( local.address(), this ) ),
      m_addr( addr ),
      m_op( op ),
      m_waitWidget( new QWaitWidget( 0 ) ),
      m_result( Failed ),
      m_busy( false ),
      m_delayedStart( false )
{
    connect( m_local, SIGNAL(pairingCreated(const QBluetoothAddress&)),
             SLOT(pairingCompleted(const QBluetoothAddress&)) );
    connect( m_local, SIGNAL(pairingFailed(const QBluetoothAddress&)),
             SLOT(pairingFailed(const QBluetoothAddress&)) );
    connect( m_local, SIGNAL(pairingRemoved(const QBluetoothAddress&)),
             SLOT(unpairingCompleted(const QBluetoothAddress&)) );

    m_waitWidget->setCancelEnabled( true );
    connect( m_waitWidget, SIGNAL(cancelled()),
             SLOT(waitWidgetCancelled()) );
}

QBluetoothPairingAgent::~QBluetoothPairingAgent()
{
    delete m_waitWidget;
}

void QBluetoothPairingAgent::showErrorDialog()
{
    QString alias = m_local->remoteAlias( m_addr );
    if ( alias.isEmpty() )
        alias = m_addr.toString();

    if ( m_op == Pair ) {
        QMessageBox::warning( 0, tr( "Pairing Error" ),
                tr( "<P>Unable to pair with %1" )
                .arg( alias ) );
    } else {
        QMessageBox::warning( 0, tr( "Pairing Error" ),
                tr( "<P>Unable to remove pairing with %1" )
                .arg( alias ) );
    }
}

/*!
    Convenience for running a pairing synchronously.
 */
QBluetoothPairingAgent::Result QBluetoothPairingAgent::pair( const QBluetoothAddress &addr, const QBluetoothLocalDevice &local )
{
    QBluetoothPairingAgent agent( Pair, addr, local );
    return agent.exec();
}

/*!
    Convenience for running an unpairing synchronously.
 */
QBluetoothPairingAgent::Result QBluetoothPairingAgent::unpair( const QBluetoothAddress &addr, const QBluetoothLocalDevice &local )
{
    QBluetoothPairingAgent agent( Unpair, addr, local );
    return agent.exec();
}

QBluetoothAddress QBluetoothPairingAgent::address() const
{
    return m_addr;
}

QBluetoothPairingAgent::Result QBluetoothPairingAgent::result() const
{
    return m_result;
}

/*!
    Starts the pairing/unpairing asynchronously. Connect to the
    pairingComplete() and unpairingComplete() signals to get the result.
 */
void QBluetoothPairingAgent::start()
{
    m_busy = true;
    m_delayedStart = false;

    m_waitWidget->setText( (m_op == Pair) ? tr("Pairing...") :
            tr("Unpairing...") );
    m_waitWidget->show();

    if ( m_op == Pair ) {
        startPairing();

    } else {
        qLog(Bluetooth) << "Start removing pairing";

        if ( m_local->removePairing( m_addr ) ) {
            qLog(Bluetooth) << "Starting unpair request";
        } else {
            qLog(Bluetooth) << "Unable to start unpairing process";
            finished( Failed );
        }
    }
}

/*!
    Runs the pairing/unpairing synchronously, and returns the result.
 */
QBluetoothPairingAgent::Result QBluetoothPairingAgent::exec()
{
    start();
    while ( m_busy )
        qApp->processEvents();
    return result();
}

void QBluetoothPairingAgent::startPairing()
{
    qLog(Bluetooth) << "QBluetoothPairingAgent::startPairing()" << m_addr.toString();

    if ( m_result == Cancelled ) {
        qLog(Bluetooth) << "QBluetoothPairingAgent: pairing cancelled before process started";
        return;
    }

    if ( m_addr.valid() && m_local->requestPairing( m_addr ) ) {
        qLog(Bluetooth) << "Starting pair request";
    } else {
        qLog(Bluetooth) << "Unable to start pairing process";
        pairingFailed( m_addr );
    }
}

void QBluetoothPairingAgent::pairingFailed( const QBluetoothAddress & )
{
    qLog(Bluetooth) << "Pairing failed" << m_local->lastError();

    // if got InProgress error, try pairing again later (user can cancel)
    if ( m_local->lastError() == QBluetoothLocalDevice::InProgress ) {
        qLog(Bluetooth) << "Device busy, delaying pairing";
        m_delayedStart = true;
        QTimer::singleShot( 200, this, SLOT(startPairing()) );
        return;
    }

    if ( m_local->lastError() == QBluetoothLocalDevice::AuthenticationCancelled )
        finished( Cancelled );
    else
        finished( Failed );
}

void QBluetoothPairingAgent::pairingCompleted( const QBluetoothAddress & )
{
    finished( Success );
}

void QBluetoothPairingAgent::unpairingCompleted( const QBluetoothAddress & )
{
    finished( Success );
}

void QBluetoothPairingAgent::waitWidgetCancelled()
{
    qLog(Bluetooth) << "QBluetoothPairingAgent::waitWidgetCancelled()";

    if ( m_delayedStart ) {
        // haven't actually started the process, so stop everything now
        finished( Cancelled );
        return;
    }

    if ( m_op == Pair ) {
        qLog(Bluetooth) << "Cancelling pairing";

        // can't do much if error while cancelling, it will just go ahead
        if ( !m_local->cancelPairing( m_addr ) )
            qLog(Bluetooth) << "Unable to cancel pairing";

        // if cancel is successful, pairingFailed() will be called
        // and device error will be AuthenticationCancelled
    } else {
        // No way to cancel unpairing, but it should finish very quickly.
        // But just in case it hangs, assume it hasn't been done and call finish
        qLog(Bluetooth) << "User wants to cancel unpairing";
        finished( Cancelled );
    }
}

void QBluetoothPairingAgent::finished( Result result )
{
    qLog(Bluetooth) << "QBluetoothPairingAgent::finished()" << m_addr.toString() << result;

    m_result = result;
    m_waitWidget->hide();

    m_busy = false;

    if ( m_op == Pair )
        emit pairingCompleted( m_addr, result );
    else
        emit unpairingCompleted( m_addr, result );
}

