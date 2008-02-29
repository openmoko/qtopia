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

#include <qpainter.h>
#include "wordpick.h"

QIMPenWordPick::QIMPenWordPick( QWidget *parent, const char *name, WFlags f )
 : QFrame( parent, name, f )
{
    clickWord = -1;
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
}

void QIMPenWordPick::clear()
{
    words.clear();
    repaint();
}

QSize QIMPenWordPick::sizeHint() const
{
    return QSize( -1, font().pixelSize()+2 );
}

void QIMPenWordPick::setWords( const QIMPenMatch::MatchWordList &w )
{
    words.clear();
    QListIterator<QIMPenMatch::MatchWord> it( w );
    for ( ; it.current(); ++it ) {
	words.append( it.current()->word );
    }
    repaint();
}

int QIMPenWordPick::onWord( QPoint p )
{
    int x = 2;
    int idx = 0;
    for ( QStringList::Iterator it = words.begin(); it != words.end(); ++it ) {
	QString word = *it;
	int w = fontMetrics().width( word );
	if ( x + w > width() )
	    break;
	if ( p.x() > x-2 && p.x() < x + w + 2 )
	    return idx;
	x += w + 5;
	if ( !idx )
	    x += 3;
	idx++;
    }

    return -1;
}

void QIMPenWordPick::paintEvent( QPaintEvent * )
{
    QPainter p(this);
    int x = 2;
    int h = p.fontMetrics().ascent() + 1;
    int idx = 0;
    for ( QStringList::Iterator it = words.begin(); it != words.end(); ++it ) {
	QString word = *it;
	int w = p.fontMetrics().width( word );
	if ( x + w > width() )
	    break;
	if ( idx == clickWord ) {
	    p.fillRect( x, 0, w, height(), black );
	    p.setPen( white );
	} else {
	    p.setPen( colorGroup().text() );
	}
	p.drawText( x, h, word );
	x += w + 5;
	if ( !idx )
	    x += 3;
	idx++;
    }
}

void QIMPenWordPick::mousePressEvent( QMouseEvent *e )
{
    clickWord = onWord( e->pos() );
    repaint();
}

void QIMPenWordPick::mouseReleaseEvent( QMouseEvent *e )
{
    int wordIdx = onWord( e->pos() );
    if ( wordIdx >= 0 && wordIdx == clickWord ) {
	//qDebug( "Clicked %s", words[wordIdx].latin1() );
	emit wordClicked( words[wordIdx] );
    }
    clickWord = -1;
    repaint();
}

