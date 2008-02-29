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
#include "sipheader.h"
#include "sipvialist.h"

SipViaList::SipViaList( void )
{
}

SipViaList::~SipViaList( void )
{
}

QString SipViaList::getViaList( void )
{
	QValueList<SipVia>::Iterator it;
	QString list;

	for( it = vialist.begin(); it != vialist.end(); ++it ) {
		list += SipHeader::getHeaderString( SipHeader::Via ) + ": " + (*it).via() + "\r\n";
	}

	return list;
}

void SipViaList::insertTopmostVia( const SipVia &newtop )
{
	vialist.prepend( newtop );
}

void SipViaList::parseVia( const QString &via )
{
	QString inputline;
	QString curvia;

	inputline = via.simplifyWhiteSpace();

	while( inputline != QString::null ) {
		if( inputline.contains( ',' ) ) {
			curvia = inputline.left( inputline.find( ',' ) );
			vialist.append( SipVia( curvia ) );
			inputline.remove( 0, inputline.find( ',' ) + 1 );
			inputline.simplifyWhiteSpace();
		} else {
			vialist.append( SipVia( inputline ) );
			inputline = QString::null;
		}
	}
}

const SipVia &SipViaList::getTopmostVia( void )
{
	return vialist[ 0 ];
}

const SipVia &SipViaList::getBottommostVia( void )
{
	return vialist.last();
}

bool SipViaList::isValid( void )
{
	QValueList<SipVia>::Iterator it;
	for( it = vialist.begin(); it != vialist.end(); ++it ) {
		if( !(*it).isValid() ) {
			return false;
		}
	}
	return true;
}

SipViaList &SipViaList::operator=( const SipViaList &v )
{
	vialist = v.vialist;
	return *this;
}

