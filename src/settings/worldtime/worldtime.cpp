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

#include "zonemap.h"
#include "worldtime.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/timestring.h>
#include <qtopia/timezone.h>
#include <qtopia/tzselect.h>
#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif


#include <qdir.h>
#include <qfile.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <stdlib.h>

WorldTime::WorldTime( QWidget *parent, const char* name,
                            WFlags fl )
    : WorldTimeBase( parent, name, fl ),
      strRealTz(0),
      bAdded(false)
{

    Config config( "qpe" );
    strRealTz = TimeZone::current().id();
    // append the labels to their respective lists...
    listCities.setAutoDelete( true );
    listTimes.setAutoDelete( true );

    listCities.append( cmdCity1 );
    listCities.append( cmdCity2 );
    listCities.append( cmdCity3 );

    listTimes.append( lblCTime1 );
    listTimes.append( lblCTime2 );
    listTimes.append( lblCTime3 );


    // kludgy way of getting the screen size so we don't have to depend
    // on a resize event...
    QWidget *d = QApplication::desktop();
    if ( d->width() < d->height() ) {
        // append for that 4 down look

        listCities.append( cmdCity4 );
        listTimes.append( lblCTime4 );

	if (d->height() >= 240) {
	    listCities.append( cmdCity5 );
	    listCities.append( cmdCity6 );
	    listTimes.append( lblCTime5 );
	    listTimes.append( lblCTime6 );
	} else {
	    cmdCity5->hide();
	    cmdCity6->hide();
	    lblCTime5->hide();
	    lblCTime6->hide();
	}

        lblCTime7->hide();
        lblCTime8->hide();
        lblCTime9->hide();
        cmdCity7->hide();
        cmdCity8->hide();
        cmdCity9->hide();
    } else {
        listCities.append( cmdCity7 );
        listCities.append( cmdCity8 );
        listCities.append( cmdCity9 );
        listTimes.append( lblCTime7 );
        listTimes.append( lblCTime8 );
        listTimes.append( lblCTime9 );

	delete lblCTime4;
        delete lblCTime5;
        delete lblCTime6;
        delete cmdCity4;
        delete cmdCity5;
        delete cmdCity6;

    }
    bAdded = true;
    readInTimes();
    changed = FALSE;
    QObject::connect( qApp, SIGNAL( clockChanged(bool) ),
                      this, SLOT( showTime() ) );
    // now start the timer so we can update the time quickly every second
    timerEvent( 0 );
}

WorldTime::~WorldTime()
{
    if ( changed ) {
	writeTimezoneChanges();
    }
}

void WorldTime::saveChanges()
{
    if (changed)
	writeTimezoneChanges();

    changed = FALSE;	
}

void WorldTime::cancelChanges()
{
    changed = FALSE;
}

void WorldTime::writeTimezoneChanges(void)
{
    qDebug("Writing changes");
    Config cfg("WorldTime");

    QListIterator<QPushButton> itCity( listCities );
    cfg.setGroup("TimeZones");

    int i;
    bool realTzWritten = FALSE;
    for ( i = 0, itCity.toFirst();  itCity.current(); i++, ++itCity ) {
	if ( !strCityTz[i].isNull() ) {
	    cfg.writeEntry("Zone"+QString::number(i), strCityTz[i]);
	    cfg.writeEntry("ZoneName"+QString::number(i), itCity.current()->text());
	    if ( strCityTz[i] == strRealTz )
		realTzWritten = TRUE;
	}
    }
    if ( realTzWritten ) {
	cfg.removeEntry("Zone"+QString::number(listCities.count()));
	cfg.removeEntry("ZoneName"+QString::number(listCities.count()));
    } else {
	cfg.writeEntry("Zone"+QString::number(listCities.count()),
	    strRealTz);
	if ( nameRealTz.isEmpty() ) {
	    int i =  strRealTz.find( '/' );
	    nameRealTz = strRealTz.mid( i+1 );
	}
	cfg.writeEntry("ZoneName"+QString::number(listCities.count()),
	    nameRealTz);
    }

    cfg.write(); // ensure that config file is written immediately
#ifndef QTOPIA_DESKTOP
#ifndef QT_NO_COP
    QCopEnvelope ( "QPE/System", "timeZoneListChange()" );
#endif
#else
    emit timeZoneListChange();
#endif

    changed = FALSE;
}

void WorldTime::timerEvent( QTimerEvent *e )
{
    if ( e )
	killTimer( timerId );
    // change the time again!!
    showTime();
    int ms = 1000 - QTime::currentTime().msec();
    timerId = startTimer( ms );
}

void WorldTime::mousePressEvent( QMouseEvent * )
{
    // DEBUG enable this to get a look at the zone information DEBUG
//    frmMap->showZones();
}

void WorldTime::showTime( void )
{
    int i;
    QListIterator<QLabel> itTime(listTimes);
    TimeZone curZone;
    QDateTime curUtcTime = TimeZone::utcDateTime(), cityTime;

    // traverse the list...
    for ( i = 0, itTime.toFirst(); itTime.current(); i++, ++itTime) {
        if ( !strCityTz[i].isNull() ) {
	    curZone = TimeZone( strCityTz[i] );
	    if ( curZone.isValid() ){
		cityTime = curZone.fromUtc( curUtcTime );
                itTime.current()->setText( TimeString::localHMDayOfWeek( cityTime ) );
	    }else{
                QMessageBox::critical( this, tr( "Time Changing" ),
                tr( "There was a problem setting timezone %1" )
                .arg( QString::number( i + 1 ) ) );
            }
        }
    }
}

void WorldTime::beginNewTz()
{
    frmMap->setFocus();
}

void WorldTime::slotNewTz( const QCString & zoneID)
{
    // determine what to do based on what putton is pressed...
    QListIterator<QPushButton> itCity(listCities);
    TimeZone curZone;
    int i = 0;
    for ( ;itCity.current(); ++itCity) {
	if ( strCityTz[i++] == zoneID.data() && !itCity.current()->isOn() )
	    return;
    }
    
    // go through the list and make adjustments based on which button is on
    for ( i = 0, itCity.toFirst(); itCity.current(); i++, ++itCity ) {
        QPushButton *cmdTmp = itCity.current();
        if ( cmdTmp->isOn() ) {
            strCityTz[i] = zoneID;
	    curZone = TimeZone( zoneID );
            cmdTmp->setText( curZone.city() );
            cmdTmp->toggle();
            // we can actually break, since there is only one button
            // that is ever pressed!
	    changed = TRUE;
            break;
        }
    }

#ifndef QTOPIA_DESKTOP
    if (changed)
	writeTimezoneChanges();
#endif

    showTime();
}

void WorldTime::readInTimes( void )
{
    Config cfg("WorldTime");
    cfg.setGroup("TimeZones");
    QListIterator<QPushButton> itCity( listCities );

    int i=0;
    nameRealTz = QString::null;
    QString zn;
    nameRealTz = "";
    for ( ; i < int(listCities.count()) ; i++ ) {
	zn = cfg.readEntry("Zone"+QString::number(i), QString::null);
	if ( zn.isNull() )
	    break;
	QString nm = cfg.readEntry("ZoneName"+QString::number(i));
	strCityTz[i] = zn;
	itCity.current()->setText(nm);
	if ( zn == strRealTz )
	    nameRealTz = nm;
	++itCity;
    }
    if ( i == 0 ) {
        // This should never be needed as the server or qtopiadesktop should always
	//     create this configuratation file if needed
	qWarning("WorldTime is recreating it's configuration using non-translated city names"); // No tr
        QStringList list = timezoneDefaults();
        int i;
        QStringList::Iterator it = list.begin();
        for ( i = 0, itCity.toFirst(); itCity.current(); i++, ++itCity ) {
            strCityTz[i] = *it++;
            itCity.current()->setText( *it++ );
        }
    }
    if ( nameRealTz.isEmpty() ) {
	//remember the current time zone even if we don't have room
	//to show it.
	zn = cfg.readEntry("Zone"+QString::number(listCities.count()),
	    QString::null);
	if ( zn == strRealTz )
	    nameRealTz = cfg.readEntry("ZoneName" +
		QString::number(listCities.count()));
	i++;
    }
}
