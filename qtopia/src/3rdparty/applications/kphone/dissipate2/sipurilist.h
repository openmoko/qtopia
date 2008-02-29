/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/*
 * Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
 * Copyright (c) 2004 Wirlab <kphone@wirlab.net>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */

#ifndef SIPURILIST_H_INCLUDED
#define SIPURILIST_H_INCLUDED

#include <qvaluelist.h>

#include "sipuri.h"

class SipUriList
{
public:
	SipUriList( void );
	SipUriList( const QString &parseinput );
	~SipUriList( void );

	QString getUriList( void );
	QString getReverseOrderList( void );

	void parseList( const QString &input );

	void removeHead( void );
	SipUri getHead( void ) const;
	SipUriList &reverseList( void );

	void addToHead( const SipUri &uri );
	void addToEnd( const SipUri &uri );

	static const SipUriList null;

	unsigned int getListLength( void ) const { return urilist.count(); }

	SipUri getPriorContact( void );

	SipUriList &operator=( const SipUriList &l );

private:
	void removeContact( SipUri uri );

	QValueList<SipUri> urilist;
};

#endif // SIPURILIST_H_INCLUDED
