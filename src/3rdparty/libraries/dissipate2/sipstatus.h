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

#ifndef SIPSTATUS_H_INCLUDED
#define SIPSTATUS_H_INCLUDED

#include <qstring.h>
#include <qdatetime.h>

const int defaultExpiresTime = 3000;  /* 50 min */
const int minValidTime = 60;          /*  1 min */

class SipStatus
{
public:
	SipStatus( unsigned int code = 0 );
	~SipStatus( void );
	void setCode( unsigned int newcode );
	void setReasonPhrase( QString newreason );
	void setExpiresTime( QDateTime dtexpirestime, bool contactExpires = false);
	void setExpiresTime( int newexpirestime, bool contactExpires = false );
	int getExpiresTime( void ) const { return expirestime; }
	int getValidTime( void );
	bool getHasExpirestime( void ) const { return hasExpirestime; }
	bool getHasContactExpirestime( void ) const { return hasContactExpirestime; }
	unsigned int getCode( void ) const { return code; }
	QString getReasonPhrase( void ) const { return reasonphrase; }
	static QString codeString( unsigned int code );
	SipStatus& operator=( const SipStatus& s );
	bool operator==( const SipStatus &s ) const;
	bool operator!=( const SipStatus &s ) const;

private:
	int expirestime;
	bool hasExpirestime;
	bool hasContactExpirestime;
	unsigned int code;
	QString reasonphrase;
};

#endif // SIPSTATUS_H_INCLUDED
