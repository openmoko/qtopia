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

#ifndef QT_NO_COP

#include "qcopenvelope_p.h"
#include <qtopianamespace.h>
#include <QBuffer>
#include <QDataStream>
#include <QFile>

#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

/*!
  \class QCopEnvelope
  \mainclass
  \brief The QCopEnvelope class encapsulates and sends QCop messages.

  This class is obsolete, and its direct equivalent is QtopiaIpcEnvelope.
  Consider using QtopiaIpcAdaptor or QtopiaServiceRequest instead of
  QtopiaIpcEnvelope, as they provide a better interface for sending
  QCop messages.

  QCop messages allow applications to communicate with each other by
  sending messages using \c QCopEnvelope and receiving messages by connecting
  to a \c QCopChannel.

  To send a message, use the following protocol:
  \code
     QCopEnvelope e(channelname, messagename);
     e << parameter1 << parameter2 << ...;
  \endcode

  For messages without parameters, simply use:
  \code
     QCopEnvelope e(channelname, messagename);
  \endcode
  where:
  \list
  \o \c{channelname} is the channel name within Qtopia, commencing with "QPE/".
  \o \c{messagename} is a function identifier followed by a list of types
  in parentheses. No white space is permitted.
  \endlist

  Note: Do not try to simplify this further as it may confuse some
  compilers.

  To receive a message either:
  \list
  \o use the predefined QPE/Application/\i{appname} channel in the application.
  For further information refer to:  QtopiaApplication::appMessage().
  \o create another channel and connect it to a slot using:
  \code
      myChannel = new QCopChannel( "QPE/FooBar", this );
      connect( myChannel, SIGNAL(received(QString,QByteArray)),
               this, SLOT(fooBarMessage(QString,QByteArray)) );
  \endcode
  \endlist
  See also: \l {Qtopia IPC Layer}{Qtopia IPC} and \l {Services}{Services}.

  \ingroup ipc

  \sa QtopiaIpcAdaptor, QtopiaServiceRequest
*/

/*!
  Constructs a QCopEnvelope to write \a message to \a channel.
  If \a message has parameters then use operator<<() to
  add the parameters to the envelope.
*/
QCopEnvelope::QCopEnvelope( const QString& channel, const QString& message ) :
    QDataStream(new QBuffer),
    ch(channel), msg(message)
{
    device()->open(QIODevice::WriteOnly);
}

/*!
  Writes the message and then destroys the QCopEnvelope.
*/
QCopEnvelope::~QCopEnvelope()
{
    QByteArray data = ((QBuffer*)device())->buffer();
    QCopChannel::send( ch, msg, data );
    delete device();
}

#endif
