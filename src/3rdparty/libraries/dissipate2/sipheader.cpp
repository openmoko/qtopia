#include "sipheader.h"

SipHeader::SipHeader( SipHeaderId newid, QString newdata )
{
	id = newid;
	data = newdata;
}

SipHeader::~SipHeader( void )
{
}

SipHeader::SipHeaderId SipHeader::matchHeader( const QString& h )
{
	QString u = h.toUpper();

	// Check short forms first
	if ( u.compare( getHeaderString( Via, true ).toUpper() ) == 0 ) { return Via; }
	if ( u.compare( getHeaderString( Content_Type, true ).toUpper() ) == 0 ) { return Content_Type; }
	if ( u.compare( getHeaderString( Content_Encoding, true ).toUpper() ) == 0 ) { return Content_Encoding; }
	if ( u.compare( getHeaderString( From, true ).toUpper() ) == 0 ) { return From; }
	if ( u.compare( getHeaderString( Call_ID, true ).toUpper() ) == 0 ) { return Call_ID; }
	if ( u.compare( getHeaderString( Contact, true ).toUpper() ) == 0 ) { return Contact; }
	if ( u.compare( getHeaderString( Content_Length, true ).toUpper() ) == 0 ) { return Content_Length; }
	if ( u.compare( getHeaderString( Subject, true ).toUpper() ) == 0 ) { return Subject; }
	if ( u.compare( getHeaderString( To, true ).toUpper() ) == 0 ) { return To; }

	if ( u.compare( getHeaderString( Accept ).toUpper() ) == 0 ) { return Accept; }
	if ( u.compare( getHeaderString( Accept_Encoding).toUpper() ) == 0 ) { return Accept_Encoding; }
	if ( u.compare( getHeaderString( Accept_Language).toUpper() ) == 0 ) { return Accept_Language; }
	if ( u.compare( getHeaderString( Allow ).toUpper() ) == 0 ) { return Allow; }
	if ( u.compare( getHeaderString( Also ).toUpper() ) == 0 ) { return Also; }
	if ( u.compare( getHeaderString( Authorization ).toUpper() ) == 0 ) { return Authorization; }
	if ( u.compare( getHeaderString( Call_ID ).toUpper() ) == 0 ) { return Call_ID; }
	if ( u.compare( getHeaderString( Contact ).toUpper() ) == 0 ) { return Contact; }
	if ( u.compare( getHeaderString( Content_Encoding ).toUpper() ) == 0 ) { return Content_Encoding; }
	if ( u.compare( getHeaderString( Content_Length ).toUpper() ) == 0 ) { return Content_Length; }
	if ( u.compare( getHeaderString( Content_Type ).toUpper() ) == 0 ) { return Content_Type; }
	if ( u.compare( getHeaderString( CSeq ).toUpper() ) == 0 ) { return CSeq; }
	if ( u.compare( getHeaderString( Date ).toUpper() ) == 0 ) { return Date; }
	if ( u.compare( getHeaderString( Encryption ).toUpper() ) == 0 ) { return Encryption; }
	if ( u.compare( getHeaderString( Expires ).toUpper() ) == 0 ) { return Expires; }
	if ( u.compare( getHeaderString( From ).toUpper() ) == 0 ) { return From; }
	if ( u.compare( getHeaderString( Hide ).toUpper() ) == 0 ) { return Hide; }
	if ( u.compare( getHeaderString( Max_Forwards ).toUpper() ) == 0 ) { return Max_Forwards; }
	if ( u.compare( getHeaderString( Organization ).toUpper() ) == 0 ) { return Organization; }
	if ( u.compare( getHeaderString( Proxy_Authenticate ).toUpper() ) == 0 ) { return Proxy_Authenticate; }
	if ( u.compare( getHeaderString( Proxy_Authorization ).toUpper() ) == 0 ) { return Proxy_Authorization; }
	if ( u.compare( getHeaderString( Proxy_Require ).toUpper() ) == 0 ) { return Proxy_Require; }
	if ( u.compare( getHeaderString( Priority ).toUpper() ) == 0 ) { return Priority; }
	if ( u.compare( getHeaderString( Require ).toUpper() ) == 0 ) { return Require; }
	if ( u.compare( getHeaderString( Retry_After ).toUpper() ) == 0 ) { return Retry_After; }
	if ( u.compare( getHeaderString( Response_Key ).toUpper() ) == 0 ) { return Response_Key; }
	if ( u.compare( getHeaderString( Record_Route ).toUpper() ) == 0 ) { return Record_Route; }
	if ( u.compare( getHeaderString( Refer_To ).toUpper() ) == 0 ) { return Refer_To; }
	if ( u.compare( getHeaderString( Referred_By ).toUpper() ) == 0 ) { return Referred_By; }
	if ( u.compare( getHeaderString( Route ).toUpper() ) == 0 ) { return Route; }
	if ( u.compare( getHeaderString( Server ).toUpper() ) == 0 ) { return Server; }
	if ( u.compare( getHeaderString( Subject ).toUpper() ) == 0 ) { return Subject; }
	if ( u.compare( getHeaderString( Timestamp ).toUpper() ) == 0 ) { return Timestamp; }
	if ( u.compare( getHeaderString( To ).toUpper() ) == 0 ) { return To; }
	if ( u.compare( getHeaderString( Unsupported ).toUpper() ) == 0 ) { return Unsupported; }
	if ( u.compare( getHeaderString( User_Agent ).toUpper() ) == 0 ) { return User_Agent; }
	if ( u.compare( getHeaderString( Via ).toUpper() ) == 0 ) { return Via; }
	if ( u.compare( getHeaderString( Warning ).toUpper() ) == 0 ) { return Warning; }
	if ( u.compare( getHeaderString( WWW_Authenticate ).toUpper() ) == 0 ) { return WWW_Authenticate; }

	if ( u.compare( getHeaderString( Event ).toUpper() ) == 0 ) { return Event; }
	if ( u.compare( getHeaderString( Allow_Events ).toUpper() ) == 0 ) { return Allow_Events; }

	return BadHeader;
}

const QString SipHeader::getHeaderString( SipHeaderId id, bool shortform )
{
	if ( shortform ) {
		switch ( id ) {
			case Accept: return "Accept";
			case Accept_Encoding: return "Accept-Encoding";
			case Accept_Language: return "Accept-Language";
			case Allow: return "Allow";
			case Allow_Events: return "Allow-Events";
			case Also: return "Also";
			case Authorization: return "Authorization";
			case Call_ID: return "i";
			case Contact: return "m";
			case Content_Encoding: return "e";
			case Content_Length: return "l";
			case Content_Type: return "c";
			case CSeq: return "CSeq";
			case Date: return "Date";
			case Encryption: return "Encryption";
			case Event: return "Event";
			case Expires: return "Expires";
			case From: return "f";
			case Hide: return "Hide";
			case Max_Forwards: return "Max-Forwards";
			case Organization: return "Organization";
			case Proxy_Authenticate: return "Proxy-Authenticate";
			case Proxy_Authorization: return "Proxy-Authorization";
			case Proxy_Require: return "Proxy-Require";
			case Priority: return "Priority";
			case Require: return "Require";
			case Retry_After: return "Retry-After";
			case Response_Key: return "Response-Key";
			case Record_Route: return "Record-Route";
			case Refer_To: return "Refer-To";
			case Referred_By: return "Referred-By";
			case Route: return "Route";
			case Server: return "Server";
			case Subject: return "s";
			case Timestamp: return "Timestamp";
			case To: return "t";
			case Unsupported: return "Unsupported";
			case User_Agent: return "User-Agent";
			case Via: return "v";
			case Warning: return "Warning";
			case WWW_Authenticate: return "WWW-Authenticate";
			case BadHeader: return "BadHeader";
		}
	} else {
		switch ( id ) {
			case Accept: return "Accept";
			case Accept_Encoding: return "Accept-Encoding";
			case Accept_Language: return "Accept-Language";
			case Allow: return "Allow";
			case Allow_Events: return "Allow-Events";
			case Also: return "Also";
			case Authorization: return "Authorization";
			case Call_ID: return "Call-ID";
			case Contact: return "Contact";
			case Content_Encoding: return "Content-Encoding";
			case Content_Length: return "Content-Length";
			case Content_Type: return "Content-Type";
			case CSeq: return "CSeq";
			case Date: return "Date";
			case Encryption: return "Encryption";
			case Event: return "Event";
			case Expires: return "Expires";
			case From: return "From";
			case Hide: return "Hide";
			case Max_Forwards: return "Max-Forwards";
			case Organization: return "Organization";
			case Proxy_Authenticate: return "Proxy-Authenticate";
			case Proxy_Authorization: return "Proxy-Authorization";
			case Proxy_Require: return "Proxy-Require";
			case Priority: return "Priority";
			case Require: return "Require";
			case Retry_After: return "Retry-After";
			case Response_Key: return "Response-Key";
			case Record_Route: return "Record-Route";
			case Refer_To: return "Refer-To";
			case Referred_By: return "Referred-By";
			case Route: return "Route";
			case Server: return "Server";
			case Subject: return "Subject";
			case Timestamp: return "Timestamp";
			case To: return "To";
			case Unsupported: return "Unsupported";
			case User_Agent: return "User-Agent";
			case Via: return "Via";
			case Warning: return "Warning";
			case WWW_Authenticate: return "WWW-Authenticate";
			case BadHeader: return "BadHeader";
		}
	}
	return QString::null;
}

