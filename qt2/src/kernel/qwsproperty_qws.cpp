/****************************************************************************
** $Id: qt/src/kernel/qwsproperty_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qwsproperty_qws.h"

#ifndef QT_NO_QWS_PROPERTIES
#include "qwscommand_qws.h"
#include "qwindowsystem_qws.h"
#include "qintdict.h"

#include <stdlib.h>
#include <stdio.h>

class QWSPropertyManagerData {
public:
    QWSPropertyManagerData()
    {
	properties.setAutoDelete( TRUE );
    }
    struct Property {
	~Property() { if ( data ) delete [] data; }
	int len;
	char *data;
    };
    Property* find( int winId, int property )
    {
	QIntDict<QWSPropertyManagerData::Property>* wp = properties.find(winId);
	if ( !wp ) return 0;
	QWSPropertyManagerData::Property* prop = wp->find(property);
	return prop;
    }

    QIntDict< QIntDict<Property> > properties;
};

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

QWSPropertyManager::QWSPropertyManager()
{
    d = new QWSPropertyManagerData;
}

QWSPropertyManager::~QWSPropertyManager()
{
    delete d;
}


//#define QWS_PROPERTY_DEBUG


bool QWSPropertyManager::setProperty( int winId, int property, int mode, const char *data, int len )
{
    QWSPropertyManagerData::Property* prop = d->find(winId,property);
    if ( !prop ) return FALSE;

    switch ( mode ) {
    case PropReplace: {
#ifdef QWS_PROPERTY_DEBUG
	qDebug( "PropReplace" );
#endif
	delete [] prop->data;
	char *nd = new char[len]; //###Must make sure this is deleted
	memcpy(nd, data, len );
	prop->len = len;
	prop->data = nd;
    } break;
    case PropAppend: {
#ifdef QWS_PROPERTY_DEBUG
	qDebug( "PropAppend" );
#endif
	int origLen = prop->len;
	char *nd = new char[ len + origLen ];
	memcpy( nd, prop->data, origLen );
	memcpy( nd+origLen, data, len );
	delete [] prop->data;
	prop->len = len + origLen;
	prop->data = nd;
    } break;
    case PropPrepend: {
#ifdef QWS_PROPERTY_DEBUG
	qDebug( "PropPrepend" );
#endif
	int origLen = origLen = prop->len;
	char *nd = new char[ len + origLen ];
	memcpy( nd, data, len );
	memcpy( nd+len, prop->data, origLen );
	delete [] prop->data;
	prop->len = len + origLen;
	prop->data = nd;
    } break;
    }
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::setProperty: %d %d (%s) to %s", winId, property, key,
	    d->properties.find( key )->data );
#endif

    return TRUE;
}

bool QWSPropertyManager::hasProperty( int winId, int property )
{
    QWSPropertyManagerData::Property* prop = d->find(winId,property);
    return !!prop;
}

bool QWSPropertyManager::removeProperty( int winId, int property )
{
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::removeProperty %d %d (%s)", winId, property, key );
#endif    
    QIntDict<QWSPropertyManagerData::Property>* wp = d->properties.find(winId);
    if ( !wp ) return FALSE;
    QWSPropertyManagerData::Property* prop = wp->find(property);
    if ( !prop ) return FALSE;
    wp->remove(property);
    if ( wp->count() == 0 )
	d->properties.remove(winId);
    return TRUE;
}

bool QWSPropertyManager::addProperty( int winId, int property )
{
    QIntDict<QWSPropertyManagerData::Property>* wp = d->properties.find(winId);
    if ( !wp ) {
	d->properties.insert(winId,wp = new QIntDict<QWSPropertyManagerData::Property>);
	wp->setAutoDelete(TRUE);
    }
    QWSPropertyManagerData::Property* prop = wp->find(property);
    if ( prop ) return FALSE;
    wp->insert(property, prop = new QWSPropertyManagerData::Property);

    prop->len = -1;
    prop->data = 0;
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::addProperty: %d %d (%s)", winId, property, key );
#endif
    return TRUE;
}

bool QWSPropertyManager::getProperty( int winId, int property, char *&data, int &len )
{
    QWSPropertyManagerData::Property* prop = d->find(winId,property);
    if ( !prop ) {
	data = 0;
	len = -1;
	return FALSE;
    }

    len = prop->len;
    data = prop->data;
#ifdef QWS_PROPERTY_DEBUG
    qDebug( "QWSPropertyManager::getProperty: %d %d (%s) %d", winId, property, 
	    key, len );
    if ( len > 0 ) {
	for ( int i = 0; i < len; i++ )
	    printf( "%c",data[i] );
	printf( "\n" );
    }
#endif    

    return TRUE;
}

bool QWSPropertyManager::removeProperties( int winId )
{
    return d->properties.remove(winId);
}

#endif //QT_NO_QWS_PROPERTIES
