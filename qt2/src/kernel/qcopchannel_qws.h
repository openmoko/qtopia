/****************************************************************************
** $Id: qt/src/kernel/qcopchannel_qws.h   2.3.12   edited 2005-10-27 $
**
** QCOP protocol classes
**
** Created : 20000616
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QCOP_H
#define QCOP_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_COP

class QCopChannelPrivate;
class QWSClient;

class QCopChannel : public QObject
{
    Q_OBJECT
public:
    QCopChannel( const QCString& channel, QObject* parent=0, const char* name=0 );
    virtual ~QCopChannel();

    QCString channel() const;

    static bool isRegistered( const QCString& channel );
    static bool send( const QCString &channel, const QCString &msg );
    static bool send( const QCString &channel, const QCString &msg,
		      const QByteArray &data );

    static void sendLocally(  const QCString &ch, const QCString &msg,
			       const QByteArray &data );

    virtual void receive( const QCString &msg, const QByteArray &data );

signals:
    void received( const QCString &msg, const QByteArray &data );

private:
    // server side
    static void registerChannel( const QString &ch, const QWSClient *cl );
    static void detach( const QWSClient *cl );
    static void answer( QWSClient *cl, const QCString &ch,
			const QCString &msg, const QByteArray &data );
    // client side
    QCopChannelPrivate* d;

    friend class QWSServer;
    friend class QApplication;
};

#endif

#endif
