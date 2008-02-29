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
#ifndef CALLLISTVIEW_H_INCLUDED
#define CALLLISTVIEW_H_INCLUDED

#include <qlist.h>
#include <qlistview.h>

#include "../dissipate2/sipcall.h"
#include "menulistview.h"
#include "qcombobox.h"

class SipCall;
class SipClient;
class CallAudio;
class KPhoneView;
class QPainter;
class QColorGroup;


class ContactsListViewItem : public QListViewItem
{
public:
	ContactsListViewItem( bool subscribe, SipCall *c, QListView *parent );
	~ContactsListViewItem( void );

	virtual QString text( int col ) const;
	virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );

	SipCall *getCall( void ) { return call; }

private:
	bool isSubscribe;
	SipCall *call;
};

class ContactsListView : public MenuListView
{
	Q_OBJECT
public:
	ContactsListView( SipClient *client, QWidget *parent = 0, const char *name = 0 );
	~ContactsListView( void );

public slots:
	void auditList( void );

private:
	SipClient *c;
	KPhoneView *v;
};


#endif // CALLLISTVIEW_H_INCLUDED
