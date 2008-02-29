/****************************************************************************
** $Id: qt/src/kernel/qwssocket_qws.h   2.3.12   edited 2005-10-27 $
**
** Definition of QWSSocket and related classes.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#ifndef QWSSOCKETDEVICE_H
#define QWSSOCKETDEVICE_H

#ifndef QT_H
#include "qsocket.h"
#include "qserversocket.h"
#endif // QT_H

#ifndef QT_NO_QWS_MULTIPROCESS

class QWSSocket : public QSocket
{
    Q_OBJECT
public:
    QWSSocket( QObject *parent=0, const char *name=0 );
   ~QWSSocket();
	        
    virtual void connectToLocalFile( const QString &file );

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWSSocket( const QWSSocket & );
    QWSSocket &operator=( const QWSSocket & );
#endif
};


class QWSServerSocket : public QServerSocket
{
    Q_OBJECT
public:
    QWSServerSocket( const QString& file, int backlog = 0,
		     QObject *parent=0, const char *name=0 );
   ~QWSServerSocket();

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWSServerSocket( const QWSServerSocket & );
    QWSServerSocket &operator=( const QWSServerSocket & );
#endif
};

#endif //QT_NO_QWS_MULTIPROCESS
#endif // QWSSOCKETDEVICE_H
