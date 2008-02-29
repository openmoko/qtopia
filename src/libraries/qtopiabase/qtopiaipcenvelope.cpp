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

#include "qtopiaipcenvelope.h"
#include <qtopianamespace.h>
#include <QBuffer>
#include <QDataStream>
#include <QFile>

#include <errno.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#include <sys/file.h>
#else
#include <stdlib.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

/*!
  \class QtopiaIpcEnvelope
  \brief The QtopiaIpcEnvelope class encapsulates and sends QCop messages
  over QtopiaChannels.

  QCop messages allow applications to communicate with each other by
  sending messages using \c QtopiaIpcEnvelope and receivng messages by connecting
  to a \c QtopiaChannel.

  To send a message, use the following protocol:
  \code
     QtopiaIpcEnvelope e(channelname, messagename);
     e << parameter1 << parameter2 << ...;
  \endcode

  For messages without parameters, simply use:
  \code
     QtopiaIpcEnvelope e(channelname, messagename);
  \endcode
  where:
  \list
  \o \c{channelname} is the channel name within Qtopia, commencing with "QPE/".
  \o \c{messagename} is a function identifier followed by a list of types
  in parentheses. No whitespace is permitted.
  \endlist

  Note: Do not try to simplify this further as it may confuse some
  compilers.

  To receive a message either:
  \list
  \o use the predefined QPE/Application/\i{appname} channel in the application.
  For further information refer to:  QtopiaApplication::appMessage().
  \o create another channel and connect it to a slot using:
  \code
      myChannel = new QtopiaChannel( "QPE/FooBar", this );
      connect( myChannel, SIGNAL(received(const QString&,const QByteArray&)),
               this, SLOT(fooBarMessage(const QString&,const QByteArray&)) );
  \endcode
  \endlist
  See also: \l {QCop Messages}{QCop Messages} and \l {Services}{Services}.

  \ingroup qtopiaemb
*/

/*!
  Constructs a QtopiaIpcEnvelope to write \a message to \a channel.
  If \a message has parameters then use operator<<() to
  add the parameters to the envelope.
*/
QtopiaIpcEnvelope::QtopiaIpcEnvelope( const QString& channel, const QString& message ) :
    QDataStream(new QBuffer),
    ch(channel), msg(message)
{
    device()->open(QIODevice::WriteOnly);
}

/*!
  Writes the message and then destroys the QtopiaIpcEnvelope.
*/
QtopiaIpcEnvelope::~QtopiaIpcEnvelope()
{
    QByteArray data = ((QBuffer*)device())->buffer();
    QtopiaChannel::send( ch, msg, data );
    delete device();
}
