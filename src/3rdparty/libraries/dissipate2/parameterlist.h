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

#ifndef PARAMETERLIST_H_INCLUDED
#define PARAMETERLIST_H_INCLUDED

#include <qlist.h>
#include <qstring.h>

class SimpleParameter
{
public:
	SimpleParameter( void );
	SimpleParameter( const SimpleParameter &p );
	SimpleParameter( QString name, QString value );
	~SimpleParameter( void );

	QString getName( void ) const { return nam; }
	QString getValue( void ) const { return val; }

	void setName( const QString &newname );
	void setValue( const QString &newval );

	SimpleParameter &operator=( const SimpleParameter &p );
private:
	QString nam;
	QString val;
};

typedef QList<SimpleParameter> ParameterList;
typedef ParameterList::Iterator ParameterListIterator;
typedef ParameterList::ConstIterator ParameterListConstIterator;

#endif // PARAMETERLIST_H_INCLUDED
