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
#ifndef KSIPPREFERENCES_H_INCLUDED
#define KSIPPREFERENCES_H_INCLUDED

#include <qtabdialog.h>

#include "../dissipate2/sipclient.h"
#include "callaudio.h"

class QLineEdit;
class QMultiLineEdit;
class QButtonGroup;
class KSipPreferences : public QTabDialog
{
	Q_OBJECT
public:
	KSipPreferences( SipClient *client, CallAudio *ca, QWidget *parent = 0, const char *name = 0 );
	~KSipPreferences( void );

protected slots:
	void slotOk( void );
	void slotApply( void );
	void slotCancel( void );
	void resetSettings( void );

private:
	enum ExpProxy { DontUseExpProxy, UseExpProxy };
	enum HideVia { NoHide, HideHop, HideRoute };
	enum CallForward { FwActive, FwInactive };
	enum Socket { UDP, TCP };
	enum Stun { stunYes, stunNo };
	enum Symmetric { symmetricYes, symmetricNo };
	enum SymMedia { symMediaYes, symMediaNo };
	
	SipClient *c;
	CallAudio *audio;
	QLineEdit *proxyaddr;
	QLineEdit *forwardaddr;
	QLineEdit *maxforwards;
	QLineEdit *phonebook;
	QLineEdit *expires;
	QLineEdit *subscribeExpires;
	QLineEdit *stunServerAddr;
	QLineEdit *stunRequestPeriod;
	QMultiLineEdit *forwardmsg;
	QMultiLineEdit *busymsg;
	QButtonGroup *expbg;
	QButtonGroup *hidebg;
	QButtonGroup *forbg;
	QButtonGroup *stun;
	QButtonGroup *symmetric;
	QButtonGroup *symMedia;
	QButtonGroup *socket;
	QLineEdit *mediaMinPort;
	QLineEdit *mediaMaxPort;
};

#endif // KSIPPREFERENCES_H_INCLUDED
