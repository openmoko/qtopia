/****************************************************************************
** $Id: qt/src/kernel/qinputcontext_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QInputContext class
**
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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
#include "qinputcontext_p.h"
#include "qstring.h"
#include "qwindowsystem_qws.h"
#include "qpaintdevice.h"
#include "qwsdisplay_qws.h"
#include "qapplication.h"

#ifndef QT_NO_QWS_IM
QWidget* QInputContext::activeWidget = 0;
QString* QInputContext::composition = 0;

bool QInputContext::activeMarkedDeleted = FALSE;

#include <qmultilineedit.h>
#include <qlineedit.h>

void QInputContext::retrieveMarkedText( QWidget *w )
{
    // may be supposed to dispatch to activeWidget instead?
    QString s;
    //Only lineedit and multilineedit are IM-enabled anyway, so
    //we might as well do it all here instead of sending events
#ifndef QT_NO_LINEEDIT
    if ( w->inherits( "QLineEdit" ) ) {
	s = ((QLineEdit*)w)->markedText();
    }
# ifndef QT_NO_MULTILINEEDIT
    else
# endif
#endif
#ifndef QT_NO_MULTILINEEDIT
    if ( w->inherits( "QMultiLineEdit" ) ) {
	s = ((QMultiLineEdit*)w)->markedText();
    }
#endif
    QByteArray ba;
    int len =  s.length()*sizeof(QChar);
    ba.duplicate( (const char*)s.unicode(), len );
    QPaintDevice::qwsDisplay()->
	setProperty( 0, QT_QWS_PROPERTY_MARKEDTEXT, 
		     QWSPropertyManager::PropReplace, ba );
}

void QInputContext::translateIMEvent( QWSIMEvent *e, QWidget *keywidget )
{
    if (activeWidget && activeMarkedDeleted) {
	if ( e->simpleData.type == QWSServer::IMEnd ) {
	    activeWidget = 0;
	    activeMarkedDeleted = FALSE;
	}
	return;
    }
    if ( e->simpleData.type == QWSServer::IMMarkedText ) {
	retrieveMarkedText( activeWidget ? activeWidget : keywidget );
	return;
    }
    
    QString txt( e->text, e->simpleData.textLen );

    if ( e->simpleData.type == QWSServer::IMCompose ) {
	//generate start event if we haven't done so already
	if ( activeWidget == 0 ) {
	    activeWidget = keywidget;
	    QIMEvent out( QEvent::IMStart, "", -1 );
	    QApplication::sendEvent( activeWidget, &out );
	    if ( !composition )
		composition = new QString;
	}

	const int cpos = QMAX(0, QMIN(e->simpleData.cpos, int(txt.length())));
	const int selLen = QMIN( e->simpleData.selLen, int(txt.length())-cpos);

	QIMComposeEvent out( QEvent::IMCompose, txt, 
			     cpos, 
			     selLen );
	QApplication::sendEvent( activeWidget, &out );

	*composition = txt;
    } else if ( e->simpleData.type == QWSServer::IMEnd ) {
	//IMEnd also known as IMInput
	//Allow multiple IMEnd events: 
	//generate start event if we haven't seen one
	//but only if we actually need to send something.

	if ( activeWidget != 0 ) {
	    QIMEvent out( QEvent::IMEnd, txt, e->simpleData.cpos );
	    QApplication::sendEvent( activeWidget, &out );
	    activeWidget = 0;
	} else if ( !txt.isEmpty() ) {
	    QIMEvent start( QEvent::IMStart, "", -1 );
	    QApplication::sendEvent( keywidget, &start );
	    QIMEvent end( QEvent::IMEnd, txt, e->simpleData.cpos );
	    QApplication::sendEvent( keywidget, &end );
	}
	if ( composition )
	    *composition = QString::null;
    } 
}

void QInputContext::reset( QWidget *newFocus )
{
    if (newFocus != activeWidget && activeWidget)  {
	// rely in IM to send an 'end'.  dont' try and guess around it,
	// let it handle its own stickyness.
	//server is obliged to send an IMEnd event in response to this call
	QPaintDevice::qwsDisplay()->resetIM();
    }
}

void QInputContext::setMicroFocusWidget(QWidget *w)
{
    if ( activeWidget && w != activeWidget )
	reset();
}

void QInputContext::notifyWidgetDeletion( QWidget *w )
{
    notifyWidgetForcedIMEnd(w);
}

QWidget *QInputContext::microFocusWidget()
{
    if (activeMarkedDeleted)
	return 0;
    return activeWidget;
}
void QInputContext::notifyWidgetForcedIMEnd( QWidget *w )
{
    /* can no longer recieve input, or may not soon, so remember to block
       drop im events that would have otherwise gone to this widget.
       */
    if ( w == activeWidget ) {
	activeMarkedDeleted = TRUE;
	reset();
    }
}

#endif //QT_NO_QWS_IM
