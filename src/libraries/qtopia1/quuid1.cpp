/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/quuid.h>

/*! 
  Returns TRUE if the left-hand QUuid is smaller than
  the right-hand QUuid.
*/
bool QUuid::operator<(const QUuid &orig) const
{
    return ( memcmp(this, &orig, sizeof(QUuid)) < 0);
}

/*! 
  Returns TRUE if the left-hand QUuid is larger than
  the right-hand QUuid.
*/

bool QUuid::operator>(const QUuid &orig) const
{
    return ( memcmp(this, &orig, sizeof(QUuid) ) > 0);
}

#if defined (Q_WS_QWS) && !defined(Q_OS_WIN32) 
/*!
  \internal
*/
QUuid::QUuid( uuid_t uuid ) 
{
    memcpy( this, uuid, sizeof(uuid_t) );
}

/*!
  \internal
*/
QUuid& QUuid::operator=(const uuid_t &orig )
{
    memcpy( this, &orig, sizeof(uuid_t) );
    return *this;
}
#endif
