/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "phonesecurity.h"

/* IN Phone mode...

   Handles pins for the SIM Card lock (e.g. access to the sim card, ability to make calls, etc),
   and Phone To Sim Lock (e.g. can access data on the phone while a different sim card is inserted).

   As these seem to be standard across all Ericssons at least.

   Other codes to handle might include:

   P2, (SIM Card 2 pin)
   CS, (Lock Control surface).

   Does NOT currently do the normal Security.conf password protection.

   GUI needs to change to 'list then item' selection.  hrm.
*/

// combo item to phone lock kind map.
PhoneLine::QueryType lockKind[] = {
    PhoneLine::LockSimCard,
    PhoneLine::LockPhoneToSimCard
};

PhoneLine::QueryType changeKind[] = {
    PhoneLine::ChangeLockSimCardPassword,
    PhoneLine::ChangeLockPhoneToSimCardPassword
};

PhoneSecurity::PhoneSecurity(QObject * parent) :
    QObject(parent),
    locktype(0)
{
    line = new PhoneLine(QString::null, this);
    connect(line, SIGNAL(notification(PhoneLine::QueryType, const QString &)),
	    this, SLOT(handleNotification(PhoneLine::QueryType, const QString &)));
    connect(line, SIGNAL(queryResult(PhoneLine::QueryType, const QString &)),
	    this, SLOT(handleQueryResult(PhoneLine::QueryType, const QString &)));
}

void PhoneSecurity::handleNotification(PhoneLine::QueryType t, const QString &v)
{
    if ( t == changeKind[0] || t == changeKind[1] ) {
	emit changed(v=="OK");
    } else if ( t == lockKind[0] || t == lockKind[1] ) {
	emit locked(v=="OK");
    }
}

void PhoneSecurity::setLockType(int t)
{
    locktype = t;
    line->query(lockKind[t]);
}

void PhoneSecurity::handleQueryResult(PhoneLine::QueryType t, const QString &v)
{
    if ( t == lockKind[locktype] ) {
	emit lockDone(v=="1");
    }
}

void PhoneSecurity::markProtected(int t, bool b, const QString& pw)
{
    locktype = t;
    line->modify(lockKind[t], (b ? "1,\"" : "0,\"")+pw+"\""); // No tr
    line->query(lockKind[t]);
}

void PhoneSecurity::changePassword(int t, const QString& old, const QString& new2)
{
    locktype = t;
    line->modify(changeKind[t], PhoneLine::formatChangePassword(old, new2));
}


