#include <stdio.h>
#include <stdlib.h>
#include <base64.h>
#include <qstringlist.h>

#include "siputil.h"
#include "extern_md5.h"
#include "sipprotocol.h"
#include "sipdebug.h"


QString dissipate_ouraddress;

Sip::Sip( void )
{
}


Sip::~Sip( void )
{
}


Sip::Method Sip::matchMethod( const QString m )
{
	if ( m.compare( getMethodString( MESSAGE ) ) == 0 ) { return MESSAGE; }
	if ( m.compare( getMethodString( INVITE ) ) == 0 ) { return INVITE; }
	if ( m.compare( getMethodString( ACK ) ) == 0 ) { return ACK; }
	if ( m.compare( getMethodString( BYE ) ) == 0 ) { return BYE; }
	if ( m.compare( getMethodString( OPTIONS ) ) == 0 ) { return OPTIONS; }
	if ( m.compare( getMethodString( CANCEL ) ) == 0 ) { return CANCEL; }
	if ( m.compare( getMethodString( REGISTER ) ) == 0 ) { return REGISTER; }
	if ( m.compare( getMethodString( MSG ) ) == 0 ) { return MSG; }
	if ( m.compare( getMethodString( SUBSCRIBE ) ) == 0 ) { return SUBSCRIBE; }
	if ( m.compare( getMethodString( NOTIFY ) ) == 0 ) { return NOTIFY; }
	if ( m.compare( getMethodString( INFO ) ) == 0 ) { return INFO; }
	if ( m.compare( getMethodString( REFER ) ) == 0 ) { return REFER; }

	return BadMethod;
}


const QString Sip::getMethodString( Method m )
{
	switch ( m ) {
		case MESSAGE: return "MESSAGE";
		case INVITE: return "INVITE";
		case ACK: return "ACK";
		case BYE: return "BYE";
		case OPTIONS: return "OPTIONS";
		case CANCEL: return "CANCEL";
		case REGISTER: return "REGISTER";
		case MSG: return "MSG";
		case SUBSCRIBE: return "SUBSCRIBE";
		case NOTIFY: return "NOTIFY";
		case INFO: return "INFO";
		case REFER: return "REFER";
		case BadMethod: return "BAD";
	};

	return QString::null;
}


QString Sip::getLocalAddress( void )
{
	if( dissipate_ouraddress == QString::null ) {
		QString s = getenv( "DISSIPATE_ADDR" );
		if( !s.isEmpty() ) {
			sip_printf( "Dissipate: Using address in DISSIPATE_ADDR\n" );
			dissipate_ouraddress = getenv( "DISSIPATE_ADDR" );
		} else {
			dissipate_ouraddress = QString::fromUtf8( SipUtil::getLocalFqdn() );
		}
	}

	return dissipate_ouraddress;
}


void Sip::setLocalAddress( const QString localaddr )
{
	dissipate_ouraddress = localaddr;
}


int Sip::parseQop(const QString &qop)
{
	QString tmp;
	int result = NO_QOP;
	if (qop == "") return result;
	QStringList lst = qop.split(",");
	QStringList::Iterator it = lst.begin();
	for(; it != lst.end(); ++it) {
		tmp = (*it).trimmed();
		if ((tmp) == "auth") result |= AUTH_QOP;
		else if ((tmp) == "auth-int") result |= AUTH_INT_QOP;
		else result |= UNKNOWN_QOP;
	}

	return result;
}


QString Sip::getDigestResponse( const QString &user, const QString &password,
			const QString &method, const QString &requri, const QString &authstr )
{
	QString realm = "";
	QString nonce = "";
	QString opaque = "";
	QString algorithm = "";
	QString qop = "";
	QString digest;
	QString cnonce;
	QString noncecount;
	QByteArray lp = authstr.toLatin1();
	const char *p = lp.constData();
	HASHHEX HA1;
	HASHHEX HA2 = "";
	HASHHEX response;
	int i = 0;
	int qop_parsed;
	while( *p ) {
		while( *p && (( *p == ' ' ) || ( *p == ',' ) || (*p == '\t') ))
			p++;
		i = 0;
		if( !*p){

/* NOP */
		}
		else if( strncasecmp( p, "realm=\"", 7 ) == 0 ) {
			p += 7;
			while( *p && p[ i ] != '"' ) {
				realm[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "nonce=\"", 7 ) == 0 ) {
			p += 7;
			while( *p && p[ i ] != '"' ) {
				nonce[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "opaque=\"", 8 ) == 0 ) {
			p += 8;
			while( *p && p[ i ] != '"' ) {
				opaque[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "algorith=\"", 10 ) == 0 ) {
			p += 10;
			while( *p && p[ i ] != '"' ) {
				algorithm[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "algorithm=\"", 11 ) == 0 ) {
			p += 11;
			while( *p && p[ i ] != '"' ) {
				algorithm[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "algorithm=", 10 ) == 0 ) {
			p += 10;
			while( *p && p[ i ] != '"' && p[ i ] != ',' &&
					p[ i ] != ' ' && p[ i ] != '\t') {
				algorithm[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "qop=\"", 5 ) == 0 ) {
			p += 5;
			while( *p && p[ i ] != '"' ) {
				qop[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "qop=", 4 ) == 0 ) {
			p += 4;
			while( *p && p[ i ] != '"' && p[ i ] != ',' &&
					p[ i ] != ' ' && p[ i ] != '\t') {
				qop[ i ] = p[ i ];
				i++;
			}
		}
		if(*p) {
		 if( i ) p += i; else p++;
		}
	}
	digest = "Digest username=\"";
	digest += user;
	digest += "\", ";
	digest += "realm=\"";
	digest += realm;
	digest += "\", ";
	digest += "nonce=\"";
	digest += nonce;
	digest += "\", ";
	digest += "uri=\"";
	digest += requri;
	digest += "\", ";
	cnonce = "abcdefghi";
	noncecount = "00000001";
	qop_parsed = parseQop(qop);
	if (qop_parsed & AUTH_QOP) sip_printf("getDigestResponse(): Remote endpoint supports Digest with qop=auth\n");
	if (qop_parsed & AUTH_INT_QOP) sip_printf("getDigestResponse(): Remote endpoint supports Digest with qop=auth-int\n");
	if (qop_parsed & UNKNOWN_QOP) sip_printf("getDigestResponse(): Unknown qop parameter found\n");
	DigestCalcHA1( "MD5", user.toLatin1().constData(), realm.toLatin1().constData(), password.toLatin1().constData(),
		nonce.toLatin1().constData(), cnonce.toLatin1().constData(), HA1 );
	sip_printf( "WL: SipProtocol: HA1=%s (%s:%s)\n",HA1,user.toLatin1().constData(),realm.toLatin1().constData() );
	DigestCalcResponse( HA1, nonce.toLatin1().constData(), noncecount.toLatin1().constData(),
		cnonce.toLatin1().constData(),
		(qop_parsed & AUTH_QOP) ? ("auth") : (""),
		method.toLatin1().constData(), requri.toLatin1().constData(), HA2, response );
	if (qop_parsed & AUTH_QOP) {
		digest += "qop=auth, ";
	}
	digest += "cnonce=\"";
	digest += cnonce;
	digest += "\", ";
	digest += "nc=";
	digest += noncecount;
	digest += ", ";
	digest += "response=\"";
	digest += response;
	digest += "\"";
	if( opaque != QString::null ) {
		digest += ", opaque=\"";
		digest += opaque;
		digest += "\"";
	}
	digest += ", algorithm=\"MD5\"";
	sip_printf( "SipProtocol: Digest calculated.\n" );

	return digest;
}


QString Sip::getBasicResponse( const QString &user, const QString &password )
{
	QString basic;
	QString userpass;
	basic = "Basic ";
	userpass += user;
	userpass += ":";
	userpass += password;
	basic += base64_encode_line( userpass.toLatin1().constData() );

	return basic;
}

