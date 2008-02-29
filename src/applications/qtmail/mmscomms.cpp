/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "mmscomms.h"
#include "account.h"
#include <QSettings>
#include <qtopialog.h>

/*!
  \class MmsComms
  \brief The MmsComms class is the base class for MMS communications.

  The MmsComms class is the base class for MMS communications.  Usually
  MMS communications via WAP is implemented by deriving from this class
  and implementing the pure virtual functions and emitting the signals
  when appropriate.
*/

/*!
  \fn void MmsComms::sendMessage(MMSMessage &msg, const QByteArray& encoded)

  Connect to the MMSC and POST the specified MMS message \a msg, the
  encoded content of which is contained in \a encoded.
  If a connection to a WAP gateway is necessary, it should be done also.
  The MailAccount details can be accessed via the \i account variable.

  \sa sendConf()
*/

/*!
  \fn void MmsComms::retrieveMessage(const QUrl &url)

  Connect to the MMSC and GET an MMS message from URL \a url.
  If a connection to a WAP gateway is necessary, it should be done also.
  The MailAccount details can be accessed via the \i account variable.

  \sa retrieveConf()
*/

/*!
  \fn bool MmsComms::isActive() const

  Return true if there are requests pending or in progress, otherwise false.
*/

/*!
  \fn void MmsComms::clearRequests()

  Clear any requests that are pending or in progress.
*/

/*!
  \fn void MmsComms::notificationInd(const MMSMessage &msg)

  This signal must be emitted when an m-notification-ind MMS message \a msg
  arrives via WAP PUSH.
*/

/*!
  \fn void MmsComms::deliveryInd(const MMSMessage &msg)

  This signal must be emitted when an m-delivery-ind MMS message \a msg
  arrives via WAP PUSH.
*/

/*!
  \fn void MmsComms::sendConf(const MMSMessage &msg)

  This signal must be emitted in response to a sendMessage(), unless
  an error occurs. The MMS message \a msg contains the confirmation details
  and the message content as received by sendMessage() .

  \sa error()
*/

/*!
  \fn void MmsComms::retrieveConf(const MMSMessage &msg, int size)

  This signal must be emitted in response to a retrieveMessage(), unless
  an error occurrs. The MMS message \a msg contains the message as decoded by retrieveMessage() which
  has a encoded size of \a size.

  \sa error()
*/

/*!
  \fn void MmsComms::statusChange(const QString &status)

  This signal can be emitted when the status of a transfer changes, e.g.
  connected to server, transferring data. The status \a status could be used to
  provide user feed back eg: a status bar.

  \sa error()
*/

/*!
  \fn void MmsComms::error(int code, const QString &msg)

  This signal must be emitted when a fatal error occurs during
  the communication process, e.g. connection refused. The code \a code should reflect
  the error seen by the underlying network protocol being used. The error message \a msg must
  be a suitable for displaying to the user and refect the meaning of \a code.
*/

/*!
  \fn void MmsComms::transferSize(int size)

  This signal must be emitted during data transfers so that the user
  can see how much data has been transferred. \a size denotes the
  amount of data transferred in bytes.
*/

/*!
  \fn void MmsComms::transfersComplete()

  This signal must be emitted when all pending or current transfers
  have completed.
*/

/*!
  Constructs a MmsComms, associated with account \a acc and having
  parent object \a parent.
*/

MmsComms::MmsComms(MailAccount *acc, QObject *parent)
    : QObject(parent), account(acc)
{
}

/*!
  Destroys this MmsComms.
*/

MmsComms::~MmsComms()
{
}

/*!
  Returns the default WAP networking account.
*/

QString MmsComms::networkConfig() const
{
    qLog(Messaging) << "Using network config" << account->networkConfig();

    return account->networkConfig();
}

