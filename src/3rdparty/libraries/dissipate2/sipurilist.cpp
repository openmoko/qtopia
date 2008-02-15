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
	QList<SipUri>::Iterator it;
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
	QList<SipUri>::Iterator it;
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
	int i = 0;

	inputline = input.simplified();

	while( i < inputline.length() ) {

		if( inputline[ i ].unicode() == 0 ) {
			break;
		} else if( inputline[ i ] == ',' ) {
			cururi = inputline.left( i );
			urilist.append( SipUri( cururi ) );
			inputline.remove( 0, i + 1 );
			inputline.simplified();
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
	urilist.erase( urilist.begin() );
}

SipUri SipUriList::getHead( void ) const
{
        if ( !urilist.isEmpty() )
	        return urilist.first();
        else
                return SipUri();
}

SipUriList &SipUriList::operator=( const SipUriList &u )
{
	urilist = u.urilist;
	return *this;
}

SipUriList &SipUriList::reverseList( void )
{
	QList<SipUri>::Iterator it;
	QList<SipUri> templist;

	for( it = urilist.begin(); it != urilist.end(); ++it ) {
		templist.prepend( (*it) );
	}

	urilist = templist;
	return *this;
}


SipUri SipUriList::getPriorContact( void )
{
	QList<SipUri>::Iterator it;
        SipUri uri;
        if ( !urilist.isEmpty() )
	        uri = urilist.first();
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
	QList<SipUri>::Iterator it;
	for( it = urilist.begin(); it != urilist.end(); ++it ) {
		if( uri == SipUri(*it) ) {
			urilist.erase( it );
			break;
		}
	}
}
