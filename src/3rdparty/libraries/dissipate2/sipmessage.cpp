#include <strings.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <qregexp.h>
#include <qdatetime.h>

#include "sipuri.h"
#include "sipvia.h"
#include "siputil.h"
#include "headerparser.h"
#include "sipmessage.h"


SipMessage::SipMessage( void )
{
	setDefaultVars();
}

SipMessage::SipMessage( const QString& parseinput )
{
	setDefaultVars();
	parseMessage( parseinput );
}

SipMessage::~SipMessage( void )
{
	qDeleteAll(headerlist.begin(), headerlist.end());
	headerlist.clear();
}

void SipMessage::setDefaultVars( void )
{
	setType( Request );
	setMethod( Sip::BadMethod );
	havebody = false;
	haveQ = false;
	lastt = 0;
	sendcount = 0;
	gettimeofday( &itimestamp, NULL );
}

QString SipMessage::getVersionString( void ) const
{
	QString ver = "SIP/2.0";
	return ver;
}

QString SipMessage::startLine( void ) const
{
	QString line;
	switch ( type ) {
		case Request:
			line += Sip::getMethodString( meth ) + " ";
			line += requesturi.reqUri() + " ";
			line += getVersionString() + "\r\n";
			break;
		case Response:
			line += getVersionString() + " ";
			line += QString::number( status.getCode() ) + " ";
			line += status.getReasonPhrase() + "\r\n";
			break;
		case BadType:
			line += "Bad Message Type";
			break;
	}

	return line;
}

QString SipMessage::messageHeaders( void )
{
	QString headers;
	SipHeader *curheader;
	headers += vialist.getViaList();
	QList<SipHeader *>::Iterator it;
	for ( it = headerlist.begin(); it != headerlist.end(); ++it ) {
		curheader = *it;
		if ( curheader->id != SipHeader::BadHeader ) {
			headers += SipHeader::getHeaderString( curheader->id ) + ": ";
			headers += curheader->data + "\r\n";
		}
	}

	if( contactlist.getListLength() > 0 ) {
		headers += SipHeader::getHeaderString( SipHeader::Contact ) + ": ";
		headers += contactlist.getUriList();
		if( meth == Sip::REGISTER ) {
			if( haveQ ) {
				headers += ";q=" + qValue;
			}
			headers += ";methods=\"INVITE, MESSAGE, INFO, SUBSCRIBE, OPTIONS, BYE, CANCEL, NOTIFY, ACK, REFER\"";
		}
		headers += "\r\n";
	}
	if( recordroute.getListLength() > 0 ) {
		headers += SipHeader::getHeaderString( SipHeader::Record_Route ) + ": ";
		headers += recordroute.getUriList();
		headers += "\r\n";
	}
	return headers;
}

QString SipMessage::messageBody( void ) const
{
	return messagebody;
}

void SipMessage::setBody( const QString& newbody )
{
	havebody = true;
	messagebody = newbody;
}

QString SipMessage::message( void )
{
	QString msg;

	msg = startLine() + messageHeaders() + "\r\n";
	if ( haveBody() ) {
		msg += messageBody();
	}

	return msg;
}

void SipMessage::parseMessage( const QString& parseinput )
{
	QString fullmessage;
	int wherecrlf;
	int wherecrlfcrlf;

	fullmessage = parseinput;

	fullmessage.replace( QRegExp( "\r\n" ), "\n" );
	fullmessage.replace( QRegExp( "\r" ), "\n" );

	wherecrlf = fullmessage.indexOf( "\n" );
	wherecrlfcrlf = fullmessage.indexOf( "\n\n" );
	if( wherecrlfcrlf == -1 ) { wherecrlfcrlf = fullmessage.length() + 1; }

	// Start line is the first line
	parseStartLine( fullmessage.left( wherecrlf ) );

	// Headers go on until \n\n
	parseHeaders( fullmessage.mid( wherecrlf, wherecrlfcrlf - wherecrlf ) );

	// The body is the rest
	setBody( fullmessage.mid( wherecrlfcrlf + 2 ) );

	if ( messageBody().length() > 0 ) {
		havebody = true;
	} else {
		havebody = false;
	}
}

void SipMessage::parseStartLine( QString startline )
{
	int lastend, i;
	QString method;
	QString requri;
	QString verstr;

	// Get method
	lastend = 0;
	while ( ( lastend < startline.length() ) && ( !( startline[ lastend ].isSpace() ) ) ) {
		lastend++;
	}
	method = startline.left( lastend );

	lastend++; // Skip the space
	i = 0;
	while ( ( ( lastend + i ) < startline.length() ) &&
			( !( startline[ lastend + i ].isSpace() ) ) ) {
		i++;
	}
	requri = startline.mid( lastend, i );

	lastend += i + 1; // Skip the space

	verstr = startline.mid( lastend ).simplified();

	if ( method == getVersionString() ) {
		// Input is a response
		setType( Response );
		status.setCode( requri.toUInt() );
		status.setReasonPhrase( verstr );
	} else {
		setType( Request );
		setMethod( Sip::matchMethod( method ) );
		setRequestUri( SipUri( requri ) );
	}
}

void SipMessage::parseHeaders( const QString &inbuf )
{
	int headerend, bodystart, bodylength;
	int lastheader;
	SipHeader::SipHeaderId id;
	QString curheader;
	QString curbody;
	QString inputline;
	int i;
	int year;
	int month;
	int day;
	int hour;
	int min;
	int sec;
	bool ok;
	QDateTime t;
	QString line;
	year = month = day = hour = min = sec = 0;
	lastheader = 0;
	for (;;) {
		if( !HeaderParser::parse( inbuf.mid( lastheader ),
			inbuf.mid( lastheader ).length(), &headerend, &bodystart, &bodylength ) ) {
			break;
		}
		curheader = inbuf.mid( lastheader, headerend ).simplified();
		curbody = inbuf.mid( lastheader + bodystart, bodylength ).simplified();
		id = SipHeader::matchHeader( curheader );
		if ( id == SipHeader::Via ) {
			vialist.parseVia( curbody );
		} else if( id == SipHeader::Expires ) {
			inputline = curbody.simplified();
			if (status.getCode() == 200) {
				if( inputline.contains( ":" ) ) {
					ok = true;
					if( inputline.contains( "Jan" ) ) {
						month = 1;
					} else if( inputline.contains( "Feb" ) ) {
						month = 2;
					} else if( inputline.contains( "Mar" ) ) {
						month = 3;
					} else if( inputline.contains( "Apr" ) ) {
						month = 4;
					} else if( inputline.contains( "May" ) ) {
						month = 5;
					} else if( inputline.contains( "Jun" ) ) {
						month = 6;
					} else if( inputline.contains( "Jul" ) ) {
						month = 7;
					} else if( inputline.contains( "Aug" ) ) {
						month = 8;
					} else if( inputline.contains( "Sep" ) ) {
						month = 9;
					} else if( inputline.contains( "Oct" ) ) {
						month = 10;
					} else if( inputline.contains( "Nov" ) ) {
						month = 11;
					} else if( inputline.contains( "Dec" ) ) {
						month = 12;
					} else {
						ok = false;
					}
					if( ok ) {
						year = inputline.mid( 12, 4 ).toInt(&ok);
					}
					if( ok ) {
						day = inputline.mid( 5, 2 ).toInt(&ok);
					}
					if( ok ) {
						hour = inputline.mid( 17, 2 ).toInt(&ok);
					}
					if( ok ) {
						min = inputline.mid( 20, 2 ).toInt(&ok);
					}
					if( ok ) {
						sec = inputline.mid( 23, 2 ).toInt(&ok);
					}
					if( ok ) {
						t = QDateTime(
							QDate( year, month, day ),
							QTime( hour, min, sec ) );
						status.setExpiresTime( t );
					}
				} else {
					sec = inputline.toInt(&ok);
					if( ok ) {
						status.setExpiresTime( sec );
					}
				}
			}
			insertHeader( id, curbody );
		} else if( id == SipHeader::Record_Route ) {
			recordroute.parseList( curbody );
			insertHeader( id, curbody );
		} else if( id == SipHeader::Contact ) {
			inputline = curbody.simplified();
			if (status.getCode() == 200) {
				if( inputline.toUpper().contains( "EXPIRES" ) &&
				    inputline.toUpper().contains( Sip::getLocalAddress() ) ) {
					if( inputline.toUpper().count( "EXPIRES" ) > 1 ) {
						i = inputline.toUpper().indexOf( Sip::getLocalAddress() );
						line = inputline.mid( i );
						if( line.contains( "," ) ) {
							i = line.toUpper().indexOf( "," );
							line = line.left( i );
						}
						i = line.toUpper().indexOf( "EXPIRES" );
						line = line.mid( i );
					} else {
						i = inputline.toUpper().indexOf( "EXPIRES" );
						line = inputline.mid( i );
					}
					if( line.toUpper().contains( "EXPIRES=\"" ) ) {
						ok = true;
						QString str;
						if( line.contains( "Jan" ) ) {
							month = 1;
						} else if( line.contains( "Feb" ) ) {
							month = 2;
						} else if( line.contains( "Mar" ) ) {
							month = 3;
						} else if( line.contains( "Apr" ) ) {
							month = 4;
						} else if( line.contains( "May" ) ) {
							month = 5;
						} else if( line.contains( "Jun" ) ) {
							month = 6;
						} else if( line.contains( "Jul" ) ) {
							month = 7;
						} else if( line.contains( "Aug" ) ) {
							month = 8;
						} else if( line.contains( "Sep" ) ) {
							month = 9;
						} else if( line.contains( "Oct" ) ) {
							month = 10;
						} else if( line.contains( "Nov" ) ) {
							month = 11;
						} else if( line.contains( "Dec" ) ) {
							month = 12;
						} else {
							ok = false;
						}
						if( ok ) {
							year = line.mid( 21, 4 ).toInt(&ok);
						}
						if( ok ) {
							day = line.mid( 14, 2 ).toInt(&ok);
						}
						if( ok ) {
							hour = line.mid( 26, 2 ).toInt(&ok);
						}
						if( ok ) {
							min = line.mid( 29, 2 ).toInt(&ok);
						}
						if( ok ) {
							sec = line.mid( 32, 2 ).toInt(&ok);
						}
						if( ok ) {
							t = QDateTime(
								QDate( year, month, day ),
								QTime( hour, min, sec ) );
							status.setExpiresTime( t, true );
						}
					} else {
						if( line.contains( ";" ) ) {
							i = line.indexOf( ";" );
							line = line.left( i );
						}
						if( line.contains( "=" ) ) {
							i = line.indexOf( "=" );
							line = line.mid( i + 1 );
						}
						sec = line.toInt(&ok);
						if( ok ) {
							status.setExpiresTime( sec );
						}
					}
				}
			}
			contactlist.parseList( curbody );
		} else {
			if ( id != SipHeader::BadHeader ) {
				insertHeader( id, curbody );
			}
		}
		lastheader += bodystart + bodylength;
		if ( lastheader >= inbuf.length() ) {
			break;
		}
	}
}

void SipMessage::insertHeader( SipHeader::SipHeaderId id, QString data )
{
	headerlist.append( new SipHeader( id, data ) );
}

bool SipMessage::hasHeader( SipHeader::SipHeaderId id )
{
	QList<SipHeader *>::Iterator it;

	for ( it = headerlist.begin(); it != headerlist.end(); ++it ) {
		if ( (*it)->id == id ) {
			return true;
		}
	}
	return false;
}

QString SipMessage::getHeaderData( SipHeader::SipHeaderId id )
{
	QList<SipHeader *>::Iterator it;

	for ( it = headerlist.begin(); it != headerlist.end(); ++it ) {
		if ( (*it)->id == id ) {
			return (*it)->data;
		}
	}
	return QString::null;
}

void SipMessage::setRequestUri( const SipUri &newrequri )
{
	requesturi = newrequri;
}

void SipMessage::setStatus( const SipStatus &stat )
{
	status = stat;
}

void SipMessage::setTimestamp( void )
{
	gettimeofday( &timestamp, NULL );
}

void SipMessage::setTimeTick( unsigned int newtt )
{
	lastt = newtt;
}

void SipMessage::incrSendCount( void )
{
	sendcount++;
}

QString SipMessage::createCallId( void )
{
	int uniqid;
	QString theid;
	struct timeval tv;
	gettimeofday( &tv, NULL );
	srand( tv.tv_usec );
	uniqid = rand();

	// fix for 3com carrier gw
	theid = QString::number( uniqid ) + "@" + Sip::getLocalAddress();

	return theid;
}

void SipMessage::setViaList( const SipViaList &copylist )
{
	vialist = copylist;
}

void SipMessage::setContactList( const SipUriList &newclist )
{
	contactlist = newclist;
}

void SipMessage::setRecordRoute( const SipUriList &newrr )
{
	recordroute = newrr;
}

void SipMessage::setQvalue( const QString& value )
{
	if( value.isEmpty() ) {
		haveQ = false;
	} else {
		haveQ = true;
		qValue = value;
	}
}

bool SipMessage::isValid( void )
{
	if( !vialist.isValid() ) {
		return false;
	}
	SipUri fromuri( getHeaderData( SipHeader::From ) );
	if( !fromuri.isValid() ) {
		return false;
	}
	SipUri touri( getHeaderData( SipHeader::To ) );
	if( !touri.isValid() ) {
		return false;
	}
	if( getMethod() == Sip::INVITE && !haveBody() ) {
		return false;
	}
	return true;
}
