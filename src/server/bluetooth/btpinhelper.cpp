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

#include "btpinhelper.h"
#include "qtopiaserverapplication.h"

#include <QtopiaApplication>
#include <QString>
#include <QByteArray>
#include <QTextStream>
#include <QMessageBox>
#include <QSettings>

#include <qpassworddialog.h>
#include <qtopiaipcenvelope.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothpasskeyrequest.h>
#include <qbluetoothremotedevice.h>
#include <qtopialog.h>

/*!
    \class BTPinHelper
    \ingroup QtopiaServer::Task::Bluetooth
    \brief The BTPinHelper class provides a default passkey agent for performing Bluetooth passkey authentications.

    The BTPinHelper class implements a Qtopia global passkey agent.  The
    internal implementation uses the QPasswordDialog to ask the user for
    the passkey.

    \sa QBluetoothPasskeyAgent
 */

/*!
    Constructs a new BTPinHelper class.  The QObject parent is given
    by \a parent.
 */
BTPinHelper::BTPinHelper(QObject *parent)
    : QBluetoothPasskeyAgent("DefaultPasskeyAgent", parent),
      m_passDialog(new QPasswordDialog)
{
    m_passDialog->setInputMode(QPasswordDialog::Plain);
    registerDefault();
}

/*!
    Destructor.
 */
BTPinHelper::~BTPinHelper()
{
    delete m_passDialog;
}

/*!
    \reimp
 */
void BTPinHelper::requestPasskey(QBluetoothPasskeyRequest &req)
{
    QBluetoothLocalDevice device(req.localDevice());
    QBluetoothRemoteDevice remote(req.remoteDevice());
    device.updateRemoteDevice(remote);

    if (m_passDialog->isVisible()) {
        qLog(Bluetooth) << "Got new pairing request for"
            << remote.address().toString()
            << "but another pairing already in progress, rejecting new pair request";
        req.setRejected();
        return;
    }

    QString msg = "<P>";
    msg += tr("You are trying to pair with %1 [%2]. Please enter a PIN. You will need to enter the same PIN on the other device.", "%1=name, %2=address")
            .arg(remote.name()).arg(remote.address().toString());

    m_passDialog->reset();
    m_passDialog->setPrompt(msg);
    if (QtopiaApplication::execDialog(m_passDialog) == QDialog::Accepted) {
        QString pairPin = m_passDialog->password();
        if ( !pairPin.isEmpty() )
            req.setPasskey(pairPin);
    } else {
        req.setRejected();
    }
}

/*!
    \reimp
 */
void BTPinHelper::cancelRequest(const QString & /*localDevice*/, const QBluetoothAddress & /*remoteAddr*/)
{
    qLog(Bluetooth) << "BTPinHelper::cancelRequest()";
    m_passDialog->reject();
}

/*!
    \reimp
 */
void BTPinHelper::release()
{

}

QTOPIA_TASK(DefaultBluetoothPassKeyAgent, BTPinHelper);
