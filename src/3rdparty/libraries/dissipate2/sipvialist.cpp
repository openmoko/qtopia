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
	QList<SipVia>::Iterator it;
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

	inputline = via.simplified();

	while( ! inputline.isEmpty() ) {
		if( inputline.contains( ',' ) ) {
			curvia = inputline.left( inputline.indexOf( ',' ) );
			vialist.append( SipVia( curvia ) );
			inputline = inputline.mid( inputline.indexOf( ',' ) + 1 );
			inputline = inputline.simplified();
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
	QList<SipVia>::Iterator it;
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

