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


// This is #included by qtopia/qpeapplication.cpp and
//                      qtopia1/qpeapplication.cpp
#ifdef Q_WS_QWS
extern Q_EXPORT QRect qt_maxWindowRect;
#endif

static void qpe_show_dialog( QDialog* d, bool nomax )
{
    QSize sh = d->sizeHint();

#ifndef QTOPIA_PHONE
    if ( d->parentWidget() && !d->parentWidget()->topLevelWidget()->isMaximized() )
	nomax = TRUE;
#endif

#ifndef Q_WS_QWS
	QSize s(qApp->desktop()->width(), qApp->desktop()->height() );
#else
	QSize s(qt_maxWindowRect.width(), qt_maxWindowRect.height() );
#endif
    QRect fg = d->frameGeometry();
    QRect cg = d->geometry();
    int frameWidth = fg.width() - cg.width();
    int maxY = s.height() - (fg.height() - cg.height());

    int h = QMAX(sh.height(),d->height());

#ifdef QTOPIA_PHONE
    if ( h >= maxY || ((!nomax) && (h > s.height()*3/5)) || d->testWFlags(Qt::WStyle_StaysOnTop)) {
	d->showMaximized();
    } else {
	int lb = cg.left()-fg.left();
	d->setGeometry(lb, qt_maxWindowRect.bottom() - h + 1,
			qApp->desktop()->width() - frameWidth, h);
	d->show();
    }
    d->raise();
#else
    int w = QMAX(sh.width(),d->width());
    int maxX = s.width() - frameWidth;

    if ( (w >= maxX && h >= maxY) || ( (!nomax) && ( w > s.width()*3/4 || h > s.height()*3/4 ) ) ) {
	d->showMaximized();
    } else {
	// try centering the dialog around its parent
	QPoint p(0,0);
	if ( d->parentWidget() ) {
	    QPoint pp = d->parentWidget()->mapToGlobal( QPoint(0,0) );
	    p = QPoint( pp.x() + d->parentWidget()->width()/2,
			pp.y() + d->parentWidget()->height()/ 2 );
	} else {
	    p = QPoint( maxX/2, maxY/2 );
	}
	
	p = QPoint( p.x() - w/2, p.y() - h/2 );
//	qDebug("p(x,y) is %d %d", p.x(), p.y() );
	
	if ( w >= maxX ) {
	    if ( p.y() < 0 )
		p.setY(0);
	    if ( p.y() + h > maxY )
		p.setY( maxY - h);
	
	    d->resize(maxX, h);
	    d->move(0, p.y() );
	} else if ( h >= maxY ) {
	    if ( p.x() < 0 )
		p.setX(0);
	    if ( p.x() + w > maxX )
		p.setX( maxX - w);
	
	    d->resize(w, maxY);
	    d->move(p.x(),0);
	} else {
	    d->resize(w, h);
	}
	
	d->show();
    }
#endif
}

