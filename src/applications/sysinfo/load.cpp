/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <stdio.h>
#include <QLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include "load.h"

LoadInfo::LoadInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    QTimer::singleShot(20, this, SLOT(init()));
}

void LoadInfo::init()
{
#ifdef SYSINFO_GEEK_MODE
    QVBoxLayout *vb = new QVBoxLayout( this );

    QString cpuInfo = getCpuInfo();
    if ( !cpuInfo.isNull() )
        vb->addWidget( new QLabel( cpuInfo, this ) );
    vb->addWidget( new Load( this ), 100 );

    QHBoxLayout *hb = new QHBoxLayout;
    vb->addLayout(hb);
    QLabel *l = new QLabel( this );
    l->setPixmap( makeLabel( Qt::red ) );
    hb->addWidget( l );
#ifndef QTOPIA_PHONE
    l = new QLabel(tr("Application CPU usage (%)"), this );
#else
    l = new QLabel(tr("Application CPU (%)"), this );
#endif
    hb->addWidget( l );
    hb->addStretch(20);

    hb = new QHBoxLayout;
    vb->addLayout(hb);
    l = new QLabel( this );
    l->setPixmap( makeLabel( Qt::green ) );
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
    p.fillRect( pm.rect(), palette().background() );
    p.fillRect( 0, 0, 20, h, Qt::black );
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
    bool haveInfo = false;
    QFile f( "/proc/cpuinfo" );
    if ( f.open( QFile::ReadOnly ) ) {
        QTextStream ts( &f );

        while ( !ts.atEnd() ) {
            QString s = ts.readLine();
            if ( s.indexOf( "model name" ) == 0 ) { // No tr
                info += s.mid( s.indexOf( ':' ) + 2 );
                haveInfo = true;
            } else if ( s.indexOf( "cpu MHz" ) == 0 ) {
                double mhz = s.mid( s.indexOf( ':' ) + 2 ).toDouble();
                info += " " + QString::number( mhz, 'f', 0 );
                info += "MHz";
                break;
            } else if ( s.indexOf( "Processor" ) == 0 ) { // No tr
                info += s.mid( s.indexOf( ':' ) + 2 );
                haveInfo = true;
                break;
#ifdef __MIPSEL__
            } else if ( s.indexOf( "cpu model" ) == 0 ) { // No tr
                info += " " + s.mid( s.indexOf( ':' ) + 2 );
                break;
            } else if ( s.indexOf( "cpu" ) == 0 ) {
                info += s.mid( s.indexOf( ':' ) + 2 );
                haveInfo = true;
#endif
            }
        }
    }

    if ( !haveInfo )
        info = QString();

#endif
    return info;
}

Load::Load( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f ), lastUser(0), lastSys(0)
{
#ifdef SYSINFO_GEEK_MODE
    setMinimumHeight( 30 );
    QPalette pal(palette());
    pal.setColor(backgroundRole(), Qt::black);
    setPalette(pal);

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
    first = true;
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

    p.setPen( Qt::gray );
    p.drawLine( 0, h - mult, width(), h - mult );
    p.drawText( 0, h - mult, "100" );
    p.drawText( 0, h, "0" );

    p.setPen( Qt::green );
    for ( int i = 1; i < points; i++ ) {
        int x1 = (i - 1) * width() / points;
        int x2 = i * width() / points;
        p.drawLine( x1, h - int(systemLoad[i-1] * mult),
                    x2, h - int(systemLoad[i] * mult) );
    }

    p.setPen( Qt::red );
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
//          scroll( -width()/points, 0, QRect( 0, 0, width() - width()/points + 1, height() ) );
            repaint();
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
        first = false;
    } else if ( tdiff < 0 ) {
        last = now;
    }
#endif
}
