/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <qretryatchat.h>
#include <qtimer.h>

/*!
    \class QRetryAtChat
    \brief The QRetryAtChat class provides a mechanism to retry an AT command until it succeeds
    \ingroup communication

    The QRetryAtChat command provides a mechanism to retry an AT command
    repeatedly until it succeeds.

    The done() signal will be emitted when the command succeeds or when
    the number of retries has been exhausted.

    \sa QAtChat
*/

class QRetryAtChatPrivate
{
public:
    QAtChat *atchat;
    QString command;
    int numRetries;
    bool deleteAfterEmit;
    int timeout;
    QTimer *timer;
};

/*!
    Construct a new retry At chat handler attached to \a parent that will
    send \a command on \a atchat for \a numRetries times before giving up.

    If \a deleteAfterEmit is true, then deleteLater() will be called
    on this object once the done() signal has been emitted.

    The \a timeout parameter specifies the amount of time to wait
    between retries (default is 1 second).
*/
QRetryAtChat::QRetryAtChat( QAtChat *atchat, const QString& command,
                            int numRetries, int timeout,
                            bool deleteAfterEmit, QObject *parent )
    : QObject( parent )
{
    d = new QRetryAtChatPrivate();
    d->atchat = atchat;
    d->command = command;
    d->numRetries = numRetries;
    d->deleteAfterEmit = deleteAfterEmit;
    d->timeout = timeout;
    d->timer = new QTimer( this );
    d->timer->setSingleShot( true );
    connect( d->timer, SIGNAL(timeout()), this, SLOT(timeout()) );
    atchat->chat( command, this, SLOT(doneInternal(bool,QAtResult)) );
}

/*!
    Destruct this retry AT chat handler.
*/
QRetryAtChat::~QRetryAtChat()
{
    delete d;
}

/*!
    \fn void QRetryAtChat::done( bool ok, const QAtResult& result )

    Signal that is emitted when the command completes successfully,
    or when the number of retries has been exhausted.  The command
    results are supplied in \a ok and \a result.
*/

void QRetryAtChat::doneInternal( bool ok, const QAtResult& result )
{
    if ( ok || d->numRetries <= 1 ) {
        emit done( ok, result );
        if ( d->deleteAfterEmit )
            deleteLater();
    } else {
        d->timer->start( d->timeout );
    }
}

void QRetryAtChat::timeout()
{
    --(d->numRetries);
    d->atchat->chat( d->command, this, SLOT(doneInternal(bool,QAtResult)) );
}
