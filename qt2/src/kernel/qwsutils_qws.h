/****************************************************************************
** $Id: qt/src/kernel/qwsutils_qws.h   2.3.12   edited 2005-10-27 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
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

#ifndef QWSUTILS_H
#define QWSUTILS_H

#ifndef QT_H
#include <qwssocket_qws.h>
#endif // QT_H

/********************************************************************
 *
 * Convenient socket functions
 *
 ********************************************************************/
#ifndef QT_NO_QWS_MULTIPROCESS
inline int qws_read_uint( QWSSocket *socket )
{
    if ( !socket || socket->size() < sizeof( int ) )
	return -1;

    int i;
    socket->readBlock( (char*)&i, sizeof( i ) );

    return i;
}

inline void qws_write_uint( QWSSocket *socket, int i )
{
    if ( !socket )
	return;

    socket->writeBlock( (char*)&i, sizeof( i ) );
}
#endif //QT_NO_QWS_MULTIPROCESS
#endif
