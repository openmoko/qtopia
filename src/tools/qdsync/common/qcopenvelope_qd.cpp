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
#ifndef Q_QDOC
#include <qcopenvelope_qd.h>
#include <qcopchannel_qd.h>

#include <QDebug>
#include <QBuffer>

/*!
  \class QCopEnvelope
  \brief The QCopEnvelope class simplifies sending messages.
  \mainclass

  The QCopEnvelope class should be used instead of the QCopChannel class when sending messages.
  It performs it's work on destruction so if you want to send a message in the middle of a method
  you should create a scope around the QCopEnvelope.

  \code
    QCopEnvelope e("QD/Connection", "setHint(QString)");
    e << "the hint is to duck";
    // send when e goes out of scope
  \endcode

  \code
    {
        QCopEnvelope e("QD/Connection", "setHint(QString)");
        e << "the hint is to duck";
        // send here
    }
  \endcode
*/

/*!
  Construct a QCopEnvelope that will be sent to the \a channel with the \a message.
  It is typical to use parameters in the message so that the receiver can pull the data out.
  Data should be inserted using the stream operator.

  \code
    QCopEnvelope e("QD/Connection", "setHint(QString)");
    e << "the hint is to duck";
  \endcode
*/
QCopEnvelope::QCopEnvelope( const QString &channel, const QString &message )
    : QDataStream(new QBuffer),
    ch(channel), msg(message)
{
    device()->open(QIODevice::WriteOnly);
}

/*!
  Send the message.
*/
QCopEnvelope::~QCopEnvelope()
{
    QByteArray data = ((QBuffer*)device())->buffer();
    QCopChannel::send(ch,msg,data);
    delete device();
}

#endif
