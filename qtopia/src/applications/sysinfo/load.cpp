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

#include <stdio.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qfile.h>
#include <qtextstream.h>
#include "load.h"

LoadInfo::LoadInfo( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
#ifdef SYSINFO_GEEK_MODE
    QVBoxLayout *vb = new QVBoxLayout( this, 6 );

    QString cpuInfo = getCpuInfo();
    if ( !cpuInfo.isNull() )
	vb->addWidget( new QLabel( cpuInfo, this ) );
    vb->addWidget( new Load( this ), 100 );
    
    QHBoxLayout *hb = new QHBoxLayout( vb );
    QLabel *l = new QLabel( this );
    l->setPixmap( makeLabel( red ) );
    hb->addWidget( l );
#ifndef QTOPIA_PHONE
    l = new QLabel(tr("Application CPU usage (%)"), this );
#else
    l = new QLabel(tr("Application CPU (%)"), this );
#endif
    hb->addWidget( l );
    hb->addStretch(20);
    
    hb = new QHBoxLayout( vb );
    l = new QLabel( this );
    l->setPixmap( makeLabel( green ) );
    hb->addWidget( l, 1 );
#ifndef QTOPIA_PHONE
    l = new QLabel(tr("System CPU usage (%)"), this );
#else
    l = new QLabel(tr("System CPU (%)"), this );
#endif
    hb->addWidget( l );
    hb->addStretch(20);
    vb->addStretch(50);
#endif
}

QPixmap LoadInfo::makeLabel( const QColor &col )
{
    int h = fontMetrics().height();
    QPixmap pm( 20 , h );
#ifdef SYSINFO_GEEK_MODE
    QPainter p( &pm );
    p.fillRect( pm.rect(), colorGroup().background() );
    p.fillRect( 0, 0, 20, h, black );
    p.setPen( col );
    p.drawLine( 2, h/2, 17, h/2 );
#else
    (void)col; //Q_UNUSED
#endif
    return pm;
}

QString LoadInfo::getCpuInfo()
{
    QString info = tr("Type: ");
#ifdef SYSINFO_GEEK_MODE
    bool haveInfo = FALSE;
    QFile f( "/proc/cpuinfo" );
    if ( f.open( IO_ReadOnly ) ) {
	QTextStream ts( &f );

	while ( !ts.atEnd() ) {
	    QString s = ts.readLine();
	    if ( s.find( "model name" ) == 0 ) { // No tr
		info += s.mid( s.find( ':' ) + 2 );
		haveInfo = TRUE;
	    } else if ( s.find( "cpu MHz" ) == 0 ) {
		double mhz = s.mid( s.find( ':' ) + 2 ).toDouble();
		info += " " + QString::number( mhz, 'f', 0 );
		info += "MHz";
		break;
	    } else if ( s.find( "Processor" ) == 0 ) { // No tr
		info += s.mid( s.find( ':' ) + 2 );
		haveInfo = TRUE;
		break;
#ifdef __MIPSEL__
	    } else if ( s.find( "cpu model" ) == 0 ) { // No tr
		info += " " + s.mid( s.find( ':' ) + 2 );
		break;
	    } else if ( s.find( "cpu" ) == 0 ) {
		info += s.mid( s.find( ':' ) + 2 );
		haveInfo = TRUE;
#endif
	    }
	}
    }

    if ( !haveInfo )
	info = QString();

#endif
    return info;
}

Load::Load( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f ), lastUser(0), lastSys(0)
{
#ifdef SYSINFO_GEEK_MODE
    setMinimumHeight( 30 );
    setBackgroundColor( black );
    points = 100;
    setMinimumWidth( points );
    userLoad = new double [points];
    systemLoad = new double [points];
    for ( int i = 0; i < points; i++ ) {
	userLoad[i] = 0.0;
	systemLoad[i] = 0.0;
    }
    maxLoad = 1.3;
    QTimer *timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), SLOT(timeout()) );
    timer->start( 2000 );
    gettimeofday( &last, 0 );
    first = TRUE;
    timeout();
#endif
}

Load::~Load()
{
#ifdef SYSINFO_GEEK_MODE
    delete [] userLoad;
    delete [] systemLoad;
#endif
}

void Load::paintEvent( QPaintEvent * )
{
#ifdef SYSINFO_GEEK_MODE
    QPainter p( this );

    int h = height() - 5;

    int mult = (int)(h / maxLoad);

    p.setPen( gray );
    p.drawLine( 0, h - mult, width(), h - mult );
    p.drawText( 0, h - mult, "100" );
    p.drawText( 0, h, "0" );

    p.setPen( green );
    for ( int i = 1; i < points; i++ ) {
	int x1 = (i - 1) * width() / points;
	int x2 = i * width() / points;
	p.drawLine( x1, h - int(systemLoad[i-1] * mult),
		    x2, h - int(systemLoad[i] * mult) );
    }

    p.setPen( red );
    for ( int i = 1; i < points; i++ ) {
	int x1 = (i - 1) * width() / points;
	int x2 = i * width() / points;
	p.drawLine( x1, h - int(userLoad[i-1] * mult),
		    x2, h - int(userLoad[i] * mult) );
    }
#endif
}

void Load::timeout()
{
#ifdef SYSINFO_GEEK_MODE
    int user;
    int usernice;
    int sys;
    int idle;
    FILE *fp;
    fp = fopen( "/proc/stat", "r" );
    fscanf( fp, "cpu %d %d %d %d", &user, &usernice, &sys, &idle );
    user += usernice;
    fclose( fp );
    struct timeval now;
    gettimeofday( &now, 0 );
    int tdiff = now.tv_usec - last.tv_usec;
    tdiff += (now.tv_sec - last.tv_sec) * 1000000;
    tdiff /= 10000;

    int udiff = user - lastUser;
    int sdiff = sys - lastSys;
    if ( tdiff > 0 ) {
	double uload = (double)udiff / (double)tdiff;
	double sload = (double)sdiff / (double)tdiff;
	if ( !first ) {
	    for ( int i = 1; i < points; i++ ) {
		userLoad[i-1] = userLoad[i];
		systemLoad[i-1] = systemLoad[i];
	    }
	    userLoad[points-1] = uload;
	    systemLoad[points-1] = sload;
//	    scroll( -width()/points, 0, QRect( 0, 0, width() - width()/points + 1, height() ) );
	    repaint( TRUE );
	    double ml = 1.3;
	    /*
	    for ( int i = 0; i < points; i++ ) {
		if ( userLoad[i] > ml )
		    ml = userLoad[i];
	    }
	    */
	    if ( maxLoad != ml ) {
		maxLoad = ml;
		update();
	    }
	}

	last = now;
	lastUser = user;
	lastSys = sys;
	first = FALSE;
    } else if ( tdiff < 0 ) {
	last = now;
    }
#endif
}
