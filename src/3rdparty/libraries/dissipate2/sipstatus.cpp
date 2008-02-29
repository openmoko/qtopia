#include <time.h>

#include "sipstatus.h"


SipStatus::SipStatus( unsigned int code )
{
	setCode( code );
	hasExpirestime = false;
	hasContactExpirestime = false;
}

SipStatus::~SipStatus( void )
{
}

void SipStatus::setCode( unsigned int newcode )
{
	code = newcode;
	setReasonPhrase( codeString( code ) );
}

void SipStatus::setExpiresTime( QDateTime dtexpirestime, bool contactExpires )
{
	if( !hasContactExpirestime ) {
		if( contactExpires ) {
			hasContactExpirestime = true;
		}
		time_t ti;
		tm *t;
		ti = time( NULL );
		t = gmtime( &ti );
		QDateTime dt(
			QDate( t->tm_year+1900, t->tm_mon + 1, t->tm_mday ),
			QTime( t->tm_hour, t->tm_min, t->tm_sec ) );
		int newexpirestime = dt.secsTo( dtexpirestime );
		if( hasExpirestime ) {
			if( newexpirestime > expirestime ) {
				return;
			}
		}
		expirestime = newexpirestime;
		hasExpirestime = true;
	}
}

void SipStatus::setExpiresTime( int newexpirestime, bool contactExpires )
{
	if( !hasContactExpirestime ) {
		if( contactExpires ) {
			hasContactExpirestime = true;
		}
		expirestime = newexpirestime;
		hasExpirestime = true;
	}
}

int SipStatus::getValidTime( void )
{
	int time;
	if( hasExpirestime ) {
		hasExpirestime = false;
		hasContactExpirestime = false;
		time = expirestime;
	} else {
		time = defaultExpiresTime;
	}
	if( time > 86400 ) {
		time = 86400;
	} else if( time < minValidTime ) {
		time = minValidTime;
	}
	return (900 * time);
}

void SipStatus::setReasonPhrase( QString newreason )
{
	reasonphrase = newreason;
}

QString SipStatus::codeString( unsigned int code )
{
	switch ( code ) {
		case 0: return "No Response";

		case 100: return "Trying";
		case 180: return "Ringing";
		case 181: return "Call is Being Forwarded";
		case 182: return "Queued";

		case 200: return "OK";

		case 300: return "Multiple Choices";
		case 301: return "Moved Permanently";
		case 302: return "Moved Temporarily";
		case 303: return "See Other";
		case 305: return "Use Proxy";
		case 380: return "Alternative Service";

		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 402: return "Payment Required";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 407: return "Proxy Authentication Required";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 413: return "Request Entity Too Large";
		case 414: return "Request-URI Too Large";
		case 415: return "Unsupported Media Type";
		case 420: return "Bad Extension";
		case 480: return "Temporarily not available";
		case 481: return "Call Leg/Transaction Does Not Exist";
		case 482: return "Loop Detected";
		case 483: return "Too Many Hops";
		case 484: return "Address Incomplete";
		case 485: return "Ambiguous";
		case 486: return "Busy Here";
		case 487: return "Request Terminated";
		case 488: return "Not Acceptable Here";

		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Time-out";
		case 505: return "SIP Version not supported";

		case 600: return "Busy Everywhere";
		case 603: return "Decline";
		case 604: return "Does not exist anywhere";
		case 606: return "Not Acceptable";
	}
	return "Unknown";
}

SipStatus& SipStatus::operator=( const SipStatus& s )
{
	setCode( s.getCode() );
	setReasonPhrase( s.getReasonPhrase() );
	hasExpirestime = s.getHasExpirestime();
	hasContactExpirestime = s.getHasContactExpirestime();
	expirestime = s.getExpiresTime();
	return *this;
}

bool SipStatus::operator==( const SipStatus &s ) const
{
	return ( getCode() == s.getCode() );
}

bool SipStatus::operator!=( const SipStatus &s ) const
{
	return !( *this == s );
}

