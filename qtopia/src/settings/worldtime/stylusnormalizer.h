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

#ifndef STYLUSNORMALIZER_H
#define STYLUSNORMALIZER_H

#include <qdatetime.h>
#include <qwidget.h>

class QTimer;

class _StylusEvent
{
public:
    _StylusEvent( const QPoint &pt = QPoint( 0, 0 ) );
    ~_StylusEvent();
    QPoint point( void ) const { return _pt; };
    QTime time( void ) const { return _t; };
    void setPoint( int x, int y) { _pt.setX( x ); _pt.setY( y ); };
    void setPoint( const QPoint &newPt ) { _pt = newPt; };
    void setTime( QTime newTime ) { _t = newTime; };

private:
    QPoint _pt;
    QTime _t;
};


class StylusNormalizer : public QWidget
{
    Q_OBJECT
public: 
    StylusNormalizer( QWidget *parent = 0, const char* name = 0 );
    ~StylusNormalizer();
    void start();
    void stop();
    void addEvent( const QPoint &pt );   // add a new point in

signals:
    void signalNewPoint( const QPoint &p );

private slots:
    void slotAveragePoint( void );  // return an averaged point

private:
    enum {SAMPLES = 10};
    _StylusEvent _ptList[SAMPLES];
    int _next;
    QTimer *_tExpire;
    bool bFirst;    // the first item added in...
};

#endif
