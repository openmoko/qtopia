/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/**********************************************************************
** This file is part of the KPhone project.
** 
** KPhone is a SIP (Session Initiation Protocol) user agent for Linux, with
** which you can initiate VoIP (Voice over IP) connections over the Internet,
** send Instant Messages and subscribe your friends' presence information.
** Read INSTALL for installation instructions, and CHANGES for the latest
** additions in functionality and COPYING for the General Public License
** (GPL).
** 
** More information about Wirlab available at http://www.wirlab.net/
** 
** Note: "This program is released under the GPL with the additional
** exemption that compiling, linking, and/or using OpenSSL is allowed."
***********************************************************************/
#include "../dissipate2/sipclient.h"
#include "callaudio.h"
#include "calllistview.h"

ContactsListViewItem::ContactsListViewItem( bool subscribe, SipCall *c, QListView *parent )
	: QListViewItem( parent ), call( c )
{
	isSubscribe = subscribe;
	QObject::connect( call, SIGNAL( callStatusUpdated() ), listView(), SLOT( triggerUpdate() ) );
}

ContactsListViewItem::~ContactsListViewItem( void )
{
	QObject::disconnect( call, 0, 0, 0 );
}

QString ContactsListViewItem::text( int col ) const
{
	switch( col ) {
		case 0: // Status
			return call->getPresenceStatus();
			break;
		case 1: // Subject
			return call->getContactStr();
			break;
		default:
			break;
	}
	return QString::null;
}

void ContactsListViewItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{
	QColorGroup _cg( cg );
	QColor c = _cg.text();
	if( isSubscribe && column == 1 ) {
		 _cg.setColor( QColorGroup::Text, Qt::darkGreen );
	}
	QListViewItem::paintCell( p, _cg, column, width, alignment );
	 _cg.setColor( QColorGroup::Text, c );
}

ContactsListView::ContactsListView( SipClient *client, QWidget *parent, const char *name )
	: MenuListView( parent, name )
{
	c = client;
	addColumn( tr("Status") );
	addColumn( tr("Contact") );
	setColumnWidth( 1, fontMetrics().maxWidth() * 9 );
	setAllColumnsShowFocus( TRUE );
	setMultiSelection( FALSE );
	auditList();
}

ContactsListView::~ContactsListView( void )
{
}

void ContactsListView::auditList( void )
{
	bool isSubscribed = false;
	clear();
	SipCallIterator it( c->getCallList() );
	SipCallIterator it2( c->getCallList() );
	it.toFirst();
	for( it.toFirst(); it.current(); ++it ) {
		if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
			isSubscribed = false;
			it2.toFirst();
			for( it2.toFirst(); it2.current(); ++it2 ) {
				if( it2.current()->getCallType() == SipCall::inSubscribeCall ) {
					if( it2.current()->getMember( it.current()->getSubject() ) ) {
						if( it2.current()->getCallStatus() == SipCall::callInProgress ) {
							isSubscribed = true;
						}
					}
				}
			}
			if( it.current()->getCallStatus() != SipCall::callDead ) {
				insertItem( new ContactsListViewItem(
					isSubscribed, it.current(), this ) );
			}
		}
	}
}
