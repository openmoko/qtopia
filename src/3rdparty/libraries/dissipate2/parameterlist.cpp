#include "parameterlist.h"


SimpleParameter::SimpleParameter( void )
{
}

SimpleParameter::SimpleParameter( const SimpleParameter& p )
{
	setName( p.getName() );
	setValue( p.getValue() );
}

SimpleParameter::SimpleParameter( QString name, QString value )
{
	nam = name;
	val = value;
}

SimpleParameter::~SimpleParameter( void )
{
}

void SimpleParameter::setName( const QString& newname )
{
	nam = newname;
}

void SimpleParameter::setValue( const QString& newval )
{
	val = newval;
}

SimpleParameter &SimpleParameter::operator=( const SimpleParameter &p )
{
	setName( p.getName() );
	setValue( p.getValue() );
	return *this;
}

