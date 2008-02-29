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

#include "vscrollview.h"
#include <qapplication.h>
#include <qobjectlist.h>
#include <qstyle.h>

VScrollView::VScrollView( QWidget *parent, const char *name)
    : QScrollView(parent, name)
{
    setHScrollBarMode( QScrollView::AlwaysOff );
    setResizePolicy( QScrollView::AutoOneFit );
    setFrameStyle(QFrame::NoFrame);
    scrollWidget = 0;
}

void VScrollView::resizeEvent(QResizeEvent *e)
{
    QScrollView::resizeEvent(e);
    setChildWidth();
}

bool VScrollView::event(QEvent* e)
{
    bool b = QScrollView::event(e);
    if ( e->type() == QEvent::LayoutHint )
	setChildWidth();
    else if ( e->type() == QEvent::Show )
	qApp->installEventFilter(this);
    else if ( e->type() == QEvent::Hide )
	qApp->removeEventFilter(this);
    return b;
}

bool VScrollView::eventFilter(QObject *o, QEvent *e)
{
    if ( !o->isWidgetType() )
	return QScrollView::eventFilter(o, e);
    QWidget *w = (QWidget*)o;
    if ( e->type() == QEvent::FocusIn && !w->isTopLevel() ) {
	// All this is just so that when our first or last focus widget
	// gains focus, we can ensureVisible.  QScrollView only handles
	// this if one of its children already has focus.
	QWidget *pw = w->parentWidget();
	while (pw && pw != viewport()) {
	    if (pw->isTopLevel())
		pw = 0;
	    else
		pw = pw->parentWidget();
	}
	if (pw) {
	    QPoint vp = w->mapTo(viewport(), QPoint(0,0));
	    QPoint cp = viewportToContents(vp);
	    ensureVisible( cp.x()+w->width()/2,
		    cp.y()+w->height()/2,
		    w->width()/2+2,
		    w->height()/2+2 );
	}
    }

    return QScrollView::eventFilter(o, e);
}

void VScrollView::setChildWidth()
{
    QWidget *ww = scrollWidget;
    if ( !ww ) {
	// User made their own child.
	if ( viewport()->children() &&
	    viewport()->children()->getFirst() &&
	    viewport()->children()->getFirst()->isWidgetType() )
		ww = (QWidget *) viewport()->children()->getFirst();
    }

    if ( ww ) {
	QSize sh = ww->sizeHint();
	int fw = width() - 2*frameWidth();
	if ( sh.height() > height() )
	    fw -= style().scrollBarExtent().width();
	ww->setFixedWidth(fw);
    }
}

/*!
  Convenience function. Call this to create-on-demand
  a plain QWidget added to the VScrollView.
*/
QWidget* VScrollView::widget()
{
    if ( !scrollWidget ) {
	scrollWidget = new QWidget(viewport());
	addChild(scrollWidget);
    }
    return scrollWidget;
}
