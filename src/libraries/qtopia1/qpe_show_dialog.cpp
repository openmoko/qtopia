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


// This is #included by qtopia/qpeapplication.cpp and
//                      qtopia1/qpeapplication.cpp

#ifdef Q_WS_QWS
extern QRect qt_maxWindowRect;
#endif

static void qpe_show_dialog( QDialog* d, bool nomax )
{
    QSize sh = d->sizeHint();
    int w = QMAX(sh.width(),d->width());
    int h = QMAX(sh.height(),d->height());

    if ( d->parentWidget() && !d->parentWidget()->topLevelWidget()->isMaximized() )
	nomax = TRUE;

#ifndef Q_WS_QWS
	QSize s(qApp->desktop()->width(), qApp->desktop()->height() );
#else
	QSize s(qt_maxWindowRect.width(), qt_maxWindowRect.height() );
#endif
    
    int maxX = s.width() - (d->frameGeometry().width() - d->geometry().width());
    int maxY = s.height() - (d->frameGeometry().height() - d->geometry().height());

    if ( (w > maxX && h > maxY) || ( (!nomax) && ( w > s.width()*3/4 || h > s.height()*3/4 ) ) ) {
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
}

