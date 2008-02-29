/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "composeim.h"

/*
   If the last 4 characters of s form a hex digit replaces them
   with the unicode character they describe.
*/
static void compose( QString &s )
{
    if ( s.length() < 4 )
	return;
    QString hex = s.right(4);
    for ( int i=0; i < 4; i++ ) {
	if ( hex[i].isSpace() )
	    return;
    }
    bool ok;
    unsigned short int unicode = hex.toUShort( &ok, 16 );
    if ( ok && unicode >= ' ' && unicode < 0xffff ) {
	s.truncate( s.length() - 4 );
	s += QChar( unicode );
    }
}

/*
   Constructs the ComposeIM and sets initial state to Off
*/
ComposeIM::ComposeIM()
{
    state = Off;
}
/*
   Destorys the ComposeIM
*/
ComposeIM::~ComposeIM() { }


/*
   Confirms current input, then resets ComposeIM and sets state to Off
*/
void ComposeIM::reset()
{
    if ( state == On ) {
	state = Off;
	sendIMEvent( QWSServer::IMEnd, composed, 0 );
	composed = "";
    }
}

/*
   Filters key events from qwsServer.  Returns FALSE if event should be
   sent on as is, otherwise returns FALSE.

   \a unicode is the text of the event. \a keycode is the Qt Key for the event
   as described in qnamespace.h. \a modifiers is a combination of zero or
   more of the following OR'ed together: Qt::ShiftButton, Qt::ControlButton
   and Qt::AltButton. If used, \a autoRepeat would be FALSE if the event
   comes from the intial press of the button, and TRUE if it comes from event
   generated from holding the button down for a period of time.
*/
bool ComposeIM::filter(int unicode, int keycode, int modifiers, 
  bool isPress, bool /*autoRepeat*/)
{
    if ( isPress && keycode == Qt::Key_Space && 
	 modifiers & Qt::ShiftButton ) {
	//switch to opposite state
	if ( state == On ) {
	    sendIMEvent( QWSServer::IMEnd, QString::null, 0 );
	    composed = "";
	    state = Off; //reset and remove text
	} else {
	    state = On;
	}
	return TRUE; //block event
    } else if ( state == On ) {
	if ( isPress ) {
	    if ( keycode == Qt::Key_Return ) {
		//accept text and remain active
		sendIMEvent( QWSServer::IMEnd, composed, composed.length() );
		composed = "";
	    } else if ( keycode == Qt::Key_Backspace ) {
		if ( composed.length() > 0 )
		    composed = composed.left( composed.length() - 1 );
		sendIMEvent( QWSServer::IMCompose, composed, composed.length(), 0 );
	    } else if ( unicode > 0 && unicode < 0xffff) {
		composed += QChar( unicode );
		compose( composed );
		sendIMEvent( QWSServer::IMCompose, composed, composed.length(), 0 );
	    }
	}
	return TRUE; //block event
    }  
    return FALSE; //pass keystroke normally.
}
