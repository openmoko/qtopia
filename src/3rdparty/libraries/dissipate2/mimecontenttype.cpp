#include "mimecontenttype.h"

const MimeContentType MimeContentType::null;

MimeContentType::MimeContentType( void )
{
}

MimeContentType::MimeContentType( const QString& initialtype, const QString& initialsubtype )
{
	m_type = initialtype;
	m_subtype = initialsubtype;
}

MimeContentType::MimeContentType( const QString& parseinput )
{
	parseContentType( parseinput );
}

MimeContentType::~MimeContentType( void )
{
}

void MimeContentType::setType( QString newtype )
{
	m_type = newtype.toLower();
}

void MimeContentType::setSubType( QString newsubtype )
{
	m_subtype = newsubtype.toLower();
}

QString MimeContentType::type( void ) const
{
	QString ctype;
	ParameterListConstIterator it;

	ctype = getType() + "/" + getSubType();

	for( it = parameters.begin(); it != parameters.end(); ++it ) {
		ctype += ";" + (*it).getName() + "=" + (*it).getValue();
	}

	return ctype;
}

void MimeContentType::setParameter( const QString &param, const QString& value )
{
	ParameterListIterator it;

	for( it = parameters.begin(); it != parameters.end(); ++it ) {
		if( param == (*it).getName() ) {
			(*it).setValue( value );
			return;
		}
	}

	parameters.append( SimpleParameter( param, value ) );
}

QString MimeContentType::queryParameter( const QString &param )
{
	ParameterListIterator it;

	for( it = parameters.begin(); it != parameters.end(); ++it ) {
		if( param == (*it).getName() ) {
			return (*it).getValue();
		}
	}

	return QString::null;
}

void MimeContentType::parseContentType( const QString& parseinput )
{
	QString inputline;
	QString curparam;
	QString attr;
	QString val;

	inputline = parseinput.simplified();

	setType( inputline.left( inputline.indexOf( "/" ) ).trimmed().toLower() );
	inputline.remove( 0, inputline.indexOf( "/" ) + 1 );

	if ( inputline.contains( ";" ) ) {
		setSubType( inputline.left( inputline.indexOf( ";" ) ).trimmed().toLower() );
		inputline.remove( 0, inputline.indexOf( ";" ) );
	} else {
		setSubType( inputline.trimmed() );
		inputline = QString::null;
	}

	while ( inputline[ 0 ] == ';' ) {
		curparam = inputline.mid( 1, inputline.indexOf( ";", 1 ) - 1 ).trimmed();
		inputline.remove( 0, inputline.indexOf( ";", 1 ) );

		if ( curparam.contains( "=" ) ) {
			attr = curparam.left( curparam.indexOf( "=" ) ).trimmed();
			val = curparam.mid( curparam.indexOf( "=" ) + 1 ).trimmed();

			parameters.append( SimpleParameter( attr, val ) );
		} else {
			parameters.append( SimpleParameter( curparam.simplified(), QString::null ) );
		}
	}
}

MimeContentType &MimeContentType::operator=( const MimeContentType &t )
{
	setType( t.getType() );
	setSubType( t.getSubType() );
	parameters = t.parameters;
	return *this;
}


bool MimeContentType::operator==( const MimeContentType &t ) const
{
	if( t.getType() == getType() && t.getSubType() == getSubType() ) {
		return true;
	}

	return false;
}

bool MimeContentType::operator==( const QString &t ) const
{
	MimeContentType mt( t );
	return ( *this == t );
}

bool MimeContentType::operator!=( const MimeContentType &t ) const
{
	return !( *this == t );
}

bool MimeContentType::operator!=( const QString &t ) const
{
	return !( *this == t );
}

