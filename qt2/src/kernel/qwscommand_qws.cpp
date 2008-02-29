/****************************************************************************
** $Id: qt/src/kernel/qwscommand_qws.cpp   2.3.12   edited 2005-10-27 $
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

#include "qwscommand_qws.h"

/*********************************************************************
 *
 * Functions to read/write commands on/from a socket
 *
 *********************************************************************/
#ifndef QT_NO_QWS_MULTIPROCESS
void qws_write_command( QWSSocket *socket, int type,
			       char *simpleData, int simpleLen, char *rawData, int rawLen )
{
    qws_write_uint( socket, type );
    qws_write_uint( socket, rawLen == -1 ? 0 : rawLen );
    if ( simpleData && simpleLen )
	socket->writeBlock( simpleData, simpleLen );
    if ( rawLen && rawData )
	socket->writeBlock( rawData, rawLen );
}

bool qws_read_command( QWSSocket *socket, char *&simpleData, int &simpleLen,
			      char *&rawData, int &rawLen,
			      int &bytesRead )
{
    if ( rawLen == -1 ) {
	if ( socket->size() < sizeof( rawLen ) )
	    return FALSE;
	rawLen = qws_read_uint( socket );
    }

    if ( !bytesRead ) {
	if ( simpleLen ) {
	    if ( socket->size() < uint(simpleLen) )
		return FALSE;
	    bytesRead = socket->readBlock( simpleData, simpleLen );
	} else
	    bytesRead = 1; // hack!
    }
	
    if ( bytesRead ) {
	if ( !rawLen )
	    return TRUE;
	if ( socket->size() < uint(rawLen) )
	    return FALSE;
	rawData = new char[ rawLen ];
	bytesRead += socket->readBlock( rawData, rawLen );
	return TRUE;
    }
    return FALSE;
}
#endif
/*********************************************************************
 *
 * QWSCommand base class - only use derived classes from that
 *
 *********************************************************************/


QWSProtocolItem::~QWSProtocolItem() { if (deleteRaw) delete [] rawDataPtr; }

#ifndef QT_NO_QWS_MULTIPROCESS
void QWSProtocolItem::write( QWSSocket *s ) {
    //qDebug( "sending type %d", type );
    qws_write_command( s, type, simpleDataPtr, simpleLen, rawDataPtr, rawLen );
}

bool QWSProtocolItem::read( QWSSocket *s ) {
    bool b = qws_read_command( s, simpleDataPtr, simpleLen,
			       rawDataPtr, rawLen, bytesRead );
    if ( b ) {
	setData(rawDataPtr, rawLen, FALSE);
	deleteRaw = TRUE;
    }
    return b;
}
#endif
void QWSProtocolItem::copyFrom( const QWSProtocolItem *item ) {
    if ( this == item )
	return;
    simpleLen = item->simpleLen;
    memcpy( simpleDataPtr, item->simpleDataPtr, simpleLen );
    setData( item->rawDataPtr, item->rawLen );
}

void QWSProtocolItem::setData( char *data, int len, bool allocateMem ) {
    if ( !data && !len ) {
	rawDataPtr = 0;
	rawLen = 0;
	return;
    }
    if ( len < 0 )
	return;
    if ( deleteRaw )
	delete [] rawDataPtr;
    if ( allocateMem ) {
	rawDataPtr = new char[ len ];
	if (data)
	    memcpy( rawDataPtr, data, len );
	deleteRaw = TRUE;
    } else {
	rawDataPtr = data;
	deleteRaw = FALSE;
    }
    rawLen = len;
}

QWSCommand *QWSCommand::factory( int type )
{
    QWSCommand *command = 0;
    switch ( type ) {
    case QWSCommand::Create:
	command = new QWSCreateCommand;
	break;
    case QWSCommand::Region:
	command = new QWSRegionCommand;
	break;
    case QWSCommand::RegionMove:
	command = new QWSRegionMoveCommand;
	break;
    case QWSCommand::RegionDestroy:
	command = new QWSRegionDestroyCommand;
	break;
    case QWSCommand::AddProperty:
	command = new QWSAddPropertyCommand;
	break;
    case QWSCommand::SetProperty:
	command = new QWSSetPropertyCommand;
	break;
    case QWSCommand::RemoveProperty:
	command = new QWSRemovePropertyCommand;
	break;
    case QWSCommand::GetProperty:
	command = new QWSGetPropertyCommand;
	break;
    case QWSCommand::SetSelectionOwner:
	command = new QWSSetSelectionOwnerCommand;
	break;
    case QWSCommand::RequestFocus:
	command = new QWSRequestFocusCommand;
	break;
    case QWSCommand::ChangeAltitude:
	command = new QWSChangeAltitudeCommand;
	break;
    case QWSCommand::DefineCursor:
	command = new QWSDefineCursorCommand;
	break;
    case QWSCommand::SelectCursor:
	command = new QWSSelectCursorCommand;
	break;
    case QWSCommand::MoveCursor:
	command = new QWSMoveCursorCommand;
	break;
    case QWSCommand::GrabMouse:
	command = new QWSGrabMouseCommand;
	break;
    case QWSCommand::GrabKeyboard:
	command = new QWSGrabKeyboardCommand;
	break;
#ifndef QT_NO_SOUND
    case QWSCommand::PlaySound:
	command = new QWSPlaySoundCommand;
	break;
#endif
#ifndef QT_NO_COP
    case QWSCommand::QCopRegisterChannel:
	command = new QWSQCopRegisterChannelCommand;
	break;
    case QWSCommand::QCopSend:
	command = new QWSQCopSendCommand;
	break;
#endif
    case QWSCommand::RegionName:
	command = new QWSRegionNameCommand;
	break;
    case QWSCommand::Identify:
	command = new QWSIdentifyCommand;
	break;
#ifndef QT_NO_QWS_IM
    case QWSCommand::SetIMInfo:
	command = new QWSSetIMInfoCommand;
	break;
    case QWSCommand::ResetIM:
	command = new QWSResetIMCommand;
	break;
    case QWSCommand::SetIMFont:
	command = new QWSSetIMFontCommand;
	break;
    case QWSCommand::IMMouse:
	command = new QWSIMMouseCommand;
	break;
#endif

    default:
	qDebug( "QWSCommand::factory : Type error - got %08x!", type );
    }
    return command;
}
