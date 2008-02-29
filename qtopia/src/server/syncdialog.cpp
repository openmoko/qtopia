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

#include "syncdialog.h"

#include <qtopia/resource.h>

#include <qimage.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qfile.h>


SyncDialog::SyncDialog( QWidget *parent, const QString &w )
    : QDialog( parent, "SyncDialog", FALSE, (WStyle_NoBorder | WStyle_Customize | WStyle_StaysOnTop) ),
    what(w), nextPt(0), rev(FALSE), hideDot(TRUE)
{
    QFont f( font() );
    f.setPointSize( 16 );
    setFont(f);

    loadPath();

    QSize ds = qApp->desktop()->size();
    setGeometry( 0, 0, ds.width(), ds.height() );
    img = Resource::loadImage( "SyncScreen" );
    if ( img.width() > ds.width() || img.height() > ds.height() ) {
	path = scalePath( path, ds.width(), img.width(), ds.height(), img.height() );
	img = img.smoothScale( ds.width(), ds.height() );
    }
    dot = Resource::loadImage( "syncdot" );
    setBackgroundColor( white );

    QPushButton *pb = new QPushButton( tr("Abort"), this, "CancelSync" );
    QSize bs = pb->sizeHint();
    bs.rwidth() += 10;
    bs.rheight() += 5;
    pb->setGeometry( (ds.width()-bs.width())/2, 4*ds.height()/5,
			bs.width(), bs.height() );
    connect( pb, SIGNAL(clicked()), this, SIGNAL(cancel()) );

    if ( path.count() >= 2 ) {
	path = generatePath( path, 8 );
	startTimer( 200 );
    }

    pb->setFocus();
}

void SyncDialog::paintEvent( QPaintEvent *pe )
{
    QPainter p(this );
    p.setClipRect( pe->rect() );
    int ox = (width() - img.width())/2;
    int oy = (height() - img.height())/2;

    QRect ir = QRect(ox, oy, img.width(), img.height()) & pe->rect();

    if ( ir.isValid() )
	p.drawImage( ir.x(), ir.y(), img, ir.x()-ox, ir.y()-oy, ir.width(), ir.height() );

    QString syncMsg = tr("Syncing:");
    p.setPen( black );
    QRect r( 0, 0, width()/2-5, QMAX(oy,80) );
    p.drawText( r, AlignRight | AlignVCenter, syncMsg );
    r.moveBy( width()/2, 0 );
    QFont f( font() );
    f.setWeight( QFont::Bold );
    p.setFont( f );
    p.drawText( r, AlignLeft | AlignVCenter, what );

    if ( !hideDot )
	p.drawImage( ox+path[nextPt].x()-dot.width()/2, oy+path[nextPt].y()-dot.height()/2, dot );
}

void SyncDialog::timerEvent( QTimerEvent * )
{
    int ox = (width() - img.width())/2;
    int oy = (height() - img.height())/2;
    int oldPt = nextPt;

    if ( !rev ) {
	nextPt++;
	if ( nextPt == (int)path.count() ) {
	    nextPt -= 2;
	    rev = TRUE;
	}
    } else {
	nextPt--;
	if ( nextPt < 0 ) {
	    nextPt = 1;
	    rev = FALSE;
	}
    }

    hideDot = FALSE;
    repaint( ox+path[nextPt].x()-dot.width()/2, oy+path[nextPt].y()-dot.height()/2,
	    dot.width(), dot.height() );
    hideDot = TRUE;
    repaint( ox+path[oldPt].x()-dot.width()/2, oy+path[oldPt].y()-dot.height()/2,
	    dot.width(), dot.height() );
}

void SyncDialog::loadPath()
{
    QString pfile = Resource::findPixmap( "syncdot" );
    if ( pfile.isEmpty() )
	return;
    int dp = pfile.findRev('.');
    pfile.replace( dp, pfile.length()-dp, ".path" );

    int count = 0;
    QFile file( pfile );
    if ( file.open( IO_ReadOnly ) ) {
	QString line;
	while ( file.readLine( line, 256 ) > 0 ) {
	    int x, y;
	    if ( sscanf( line.latin1(), "%d %d", &x, &y ) == 2 ) {
		path.resize( count+1 );
		path[count++] = QPoint(x, y);
	    }
	}
    }
}

QPointArray SyncDialog::scalePath( const QPointArray &pa, int xn, int xd, int yn, int yd )
{
    QPointArray sa( pa.size() );

    for ( unsigned i = 0; i < pa.count(); i++ ) {
	int x = xn * pa[int(i)].x() / xd;
	int y = yn * pa[int(i)].y() / yd;
	sa[int(i)] = QPoint( x, y );
    }

    return sa;
}

QPointArray SyncDialog::generatePath( const QPointArray &pa, int dist )
{
    if ( pa.count() < 2 )
	return pa;

    QPointArray fa;
    int count = 0;
    fa.resize( count+1 );
    fa[count++] = pa[0];
    for ( unsigned i = 0; i < pa.count()-1; i++ ) {
	int x1 = pa[int(i)].x();
	int y1 = pa[int(i)].y();
	int x2 = pa[int(i+1)].x();
	int y2 = pa[int(i+1)].y();
	int dx = x2 - x1;
	int dy = y2 - y1;
	int pts = (QMAX(QABS(dx),QABS(dy)) + dist/2 )/dist;
	for ( int j = 1; j < pts; j++ ) {
	    int x = j * dx / pts;
	    int y = j * dy / pts;
	    fa.resize( count+1 );
	    fa[count++] = pa[int(i)] + QPoint( x, y );
	}
	fa.resize( count+1 );
	fa[count++] = pa[int(i+1)];
    }

    return fa;
}

