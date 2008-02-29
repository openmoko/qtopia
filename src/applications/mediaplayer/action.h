/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef ACTION_H
#define ACTION_H


#include <qaction.h>
#include <qslider.h>
#include <qtoolbutton.h>
#include <qtopia/resource.h>
#include <qtopia/qpemenubar.h>
#include <qtopia/qpetoolbar.h>


class Action : public QAction {
    Q_OBJECT
public:
    Action( QWidget *parent, const QString& text, const QString& icon, QObject *handler, const QString& slot, bool t = FALSE )
	: QAction( text, Resource::loadIconSet( icon ), QString::null, 0, parent ) {
	setToggleAction( t );
	connect( this, t ? SIGNAL( toggled(bool) ) : SIGNAL( activated() ), handler, slot );
    }
public slots:
    void setOn2( bool b ) { setOn( b ); }
};


class MySlider : public QSlider {
public:
    MySlider( Orientation orientation, QWidget *parent ) : QSlider( orientation, parent ) { } 
    void mousePressEvent( QMouseEvent *e ) {
	// Turn left buttons in to middle buttons!
	ButtonState bs1 = ( e->button() == LeftButton ) ? MidButton : e->button();
	ButtonState bs2 = ( e->state() == LeftButton ) ? MidButton : e->state();
	QMouseEvent me( QEvent::MouseButtonPress, e->pos(), e->globalPos(), bs1, bs2 );
	QSlider::mousePressEvent( &me );
	emit sliderPressed();
    };
    void mouseReleaseEvent( QMouseEvent *e ) {
	// Turn left buttons in to middle buttons!
	ButtonState bs1 = ( e->button() == LeftButton ) ? MidButton : e->button();
	ButtonState bs2 = ( e->state() == LeftButton ) ? MidButton : e->state();
	QMouseEvent me( QEvent::MouseButtonPress, e->pos(), e->globalPos(), bs1, bs2 );
	QSlider::mouseReleaseEvent( &me );
	emit sliderReleased();
    };
};



#endif

