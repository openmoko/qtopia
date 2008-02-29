/****************************************************************************
** $Id: qt/src/kernel/qwsevent_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of Qt/Embedded events
**
** Created : 000101
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#include "qwsevent_qws.h"

QWSEvent *QWSEvent::factory( int type )
{
    QWSEvent *event = 0;
    switch ( type ) {
    case QWSEvent::Connected:
	event = new QWSConnectedEvent;
	break;
    case QWSEvent::MaxWindowRect:
	event = new QWSMaxWindowRectEvent;
	break;
    case QWSEvent::Mouse:
	event = new QWSMouseEvent;
	break;
    case QWSEvent::Focus:
	event = new QWSFocusEvent;
	break;
    case QWSEvent::Key:
	event = new QWSKeyEvent;
	break;
    case QWSEvent::RegionModified:
	event = new QWSRegionModifiedEvent;
	break;
    case QWSEvent::Creation:
	event = new QWSCreationEvent;
	break;
#ifndef QT_NO_QWS_PROPERTIES
    case QWSEvent::PropertyNotify:
	event = new QWSPropertyNotifyEvent;
	break;
    case QWSEvent::PropertyReply:
	event = new QWSPropertyReplyEvent;
	break;
#endif // QT_NO_QWS_PROPERTIES	
    case QWSEvent::SelectionClear:
	event = new QWSSelectionClearEvent;
	break;
    case QWSEvent::SelectionRequest:
	event = new QWSSelectionRequestEvent;
	break;
    case QWSEvent::SelectionNotify:
	event = new QWSSelectionNotifyEvent;
	break;
#ifndef QT_NO_COP
    case QWSEvent::QCopMessage:
	event = new QWSQCopMessageEvent;
	break;
#endif
    case QWSEvent::WindowOperation:
	event = new QWSWindowOperationEvent;
	break;
#ifndef QT_NO_QWS_IM
    case QWSEvent::IMEvent:
	event = new QWSIMEvent;
	break;
#endif
    default:
	qDebug( "QWSDisplayData::readMore() : Protocol error - got %08x!", type );
    }
    return event;
}

