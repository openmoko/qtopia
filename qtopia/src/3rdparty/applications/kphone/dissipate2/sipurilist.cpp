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
#include "sipurilist.h"

const SipUriList SipUriList::null;

SipUriList::SipUriList( void )
{
}

SipUriList::SipUriList( const QString &parseinput )
{
	parseList( parseinput );
}

SipUriList::~SipUriList( void )
{
}

QString SipUriList::getUriList( void )
{
	QValueList<SipUri>::Iterator it;
	QString list;
	bool first = true;

	for( it = urilist.begin(); it != urilist.end(); ++it ) {
		if( !first ) {
			list += ", ";
		}

		list += (*it).nameAddr();
		first = false;
	}
	return list;
}

QString SipUriList::getReverseOrderList( void )
{
	QValueList<SipUri>::Iterator it;
	QString list;
	bool first = true;

	for( it = urilist.end(); it != urilist.begin(); --it ) {
		if( !first ) {
			list += ", ";
		}

		list += (*it).nameAddr();
		first = false;
	}
	return list;
}

void SipUriList::parseList( const QString &input )
{
	QString inputline;
	QString cururi;
	unsigned int i = 0;

	inputline = input.simplifyWhiteSpace();

	while( i < inputline.length() ) {

		if( inputline[ i ] == QChar::null ) {
			break;
		} else if( inputline[ i ] == ',' ) {
			cururi = inputline.left( i );
			urilist.append( SipUri( cururi ) );
			inputline.remove( 0, i + 1 );
			inputline.simplifyWhiteSpace();
			i = 0;
		} else if( inputline[ i ] == '<' ) {
			while( i < inputline.length() && inputline[ i ] != '>' ) i++;
		} else if( inputline[ i ] == '\"' ) {
			i++;
			while( i < inputline.length() && inputline[ i ] != '\"' ) i++;
			i++;
		} else {
			i++;
		}
	}

	if( i > 0 ) urilist.append( SipUri( inputline ) );
}

void SipUriList::addToHead( const SipUri &uri )
{
	urilist.prepend( uri );
}

void SipUriList::addToEnd( const SipUri &uri )
{
	urilist.append( uri );
}

void SipUriList::removeHead( void )
{
	urilist.remove( urilist.begin() );
}

SipUri SipUriList::getHead( void ) const
{
	return urilist.first();
}

SipUriList &SipUriList::operator=( const SipUriList &u )
{
	urilist = u.urilist;
	return *this;
}

SipUriList &SipUriList::reverseList( void )
{
	QValueList<SipUri>::Iterator it;
	QValueList<SipUri> templist;

	for( it = urilist.begin(); it != urilist.end(); ++it ) {
		templist.prepend( (*it) );
	}

	urilist = templist;
	return *this;
}


SipUri SipUriList::getPriorContact( void )
{
	QValueList<SipUri>::Iterator it;
	SipUri uri = urilist.first();
	int q = 0;
	for( it = urilist.begin(); it != urilist.end(); ++it ) {
		SipUri contact = SipUri(*it);
		if( q < contact.getPrior().toInt() ) {
			q = contact.getPrior().toInt();
			uri = contact;
		}
	}
	removeContact( uri );
	
	return uri;
}

void SipUriList::removeContact( SipUri uri )
{
	QValueList<SipUri>::Iterator it;
	for( it = urilist.begin(); it != urilist.end(); ++it ) {
		if( uri == SipUri(*it) ) {
			urilist.remove( it );
			break;
		}
	}
}
