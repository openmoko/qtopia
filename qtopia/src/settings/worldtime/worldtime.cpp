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

#include "zonemap.h"
#include "worldtime.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/timestring.h>
#include <qtopia/timezone.h>
#include <qtopia/resource.h>
#include <qtopia/tzselect.h>
#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif
#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#include <qaction.h>
#endif


#include <qdir.h>
#include <qfile.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qwidgetstack.h>

#include <stdlib.h>

WorldTime::WorldTime( QWidget *parent, const char* name,
                            WFlags fl )
    : QWidget( parent, name, fl|Qt::WStyle_ContextHelp ),
      strRealTz(0),
      bAdded(false)
{


#ifdef Q_WS_QWS
    setCaption(tr("World Time"));
#endif
    listCities.setAutoDelete( true );
    listTimes.setAutoDelete( true );

    // checkSize;
    int columns, rows;
    QSize dsize = QApplication::desktop()->size();
    if (dsize.height() < 300) {
	mMode = Minimal;
	columns = 2;
	rows = 3;
    } else {
	if ( dsize.width() < dsize.height() ) {
	    mMode = Tall;
	    columns = 2;
	    rows = 7;
	} else {
	    mMode = Wide;
	    columns = 4;
	    rows = 4;
	}
    }

    // first, work out current number of columns?
    QGridLayout *gl = new QGridLayout(this, rows, columns);
    frmMap = new ZoneMap(this);
    QSizePolicy sp = frmMap->sizePolicy();
    sp.setHeightForWidth(TRUE);
    sp.setVerData(QSizePolicy::Preferred);
    sp.setHorData(QSizePolicy::Preferred);
    frmMap->setSizePolicy(sp);
    gl->addMultiCellWidget(frmMap, 0, 0, 0, columns-1);

    if (mMode == Minimal) {
	// all in widgetstack
	// remember, hboxes and ...
	mStack = new QWidgetStack(this);
	mCombo = new QComboBox(this);
#ifndef QTOPIA_PHONE
	QToolButton *tb = new QToolButton(this);
	tb->setIconSet(Resource::loadIconSet("edit"));

	connect(tb, SIGNAL(clicked()), this, SLOT(beginNewTz()));

	gl->addWidget(tb, 1, 1);
	gl->addWidget(mCombo, 1, 0);
#else
	gl->addMultiCellWidget(mCombo, 1, 1, 0, 1);
#endif
	gl->addMultiCellWidget(mStack, 2, 2, 0, 1, AlignTop);
	gl->setRowStretch(0, 0);
	gl->setRowStretch(1, 0);
	gl->setRowStretch(2, 1);

	for (int i = 0; i < CITIES; i++) {
	    listTimes.append(new CityInfo(mStack));

	    mStack->addWidget(listTimes.at(i), i);
	}
	connect(mCombo, SIGNAL(activated(int)), mStack, SLOT(raiseWidget(int)));
	mStack->raiseWidget(0);
    } else {
	int i, j;
	mStack = 0;
	mCombo = 0;
	for (i = 0; i < CITIES; i++) {
	    listCities.append(new QPushButton(this));
	    listCities.at(i)->setIconSet(Resource::loadIconSet("edit"));
	    listCities.at(i)->setToggleButton(TRUE);

	    connect(listCities.at(i), SIGNAL(clicked()), this, SLOT(beginNewTz()));

	    listTimes.append(new CityInfo(this));

	    if (mMode == Tall) {
		gl->addWidget(listCities.at(i),i+1, 0, i==5 ? AlignTop : 0);
		gl->addWidget(listTimes.at(i), i+1, 1, i==5 ? AlignTop : 0);
	    } else {
		gl->addWidget(listCities.at(i),(i+2)/2, 2*(i%2), i >= 4 ? AlignTop : 0);
		gl->addWidget(listTimes.at(i),(i+2)/2, 2*(i%2)+1, i >= 4 ? AlignTop : 0);
	    }
	    gl->setRowStretch(0, 0);
	    gl->setRowStretch(1, 0);
	    gl->setRowStretch(2, 2);
	    if (mMode == Tall) {
		gl->setRowStretch(3, 0);
		gl->setRowStretch(4, 0);
		gl->setRowStretch(5, 0);
		gl->setRowStretch(6, 1);
	    } else {
		gl->setRowStretch(3, 1);
	    }
	}
	for (i = 0; i < CITIES; i++)
	    for (j = 0; j < CITIES; j++)
		if (i!= j)
		    connect(listCities.at(i),
			    SIGNAL(toggled(bool)),
			    listCities.at(j),
			    SLOT(setDisabled(bool)));

    }

#ifdef QTOPIA_PHONE
    ContextMenu *contextMenu = new ContextMenu(this);
    if (mMode == Minimal) {
	QAction *a = new QAction(tr("Select City"),
		Resource::loadIconSet("edit"), QString::null, 0, this, 0);
	connect(a, SIGNAL(activated()), this, SLOT(beginNewTz()));
	a->addTo(contextMenu);
    }
#endif

    Config config( "qpe" );
    strRealTz = TimeZone::current().id();
    // append the labels to their respective lists...

    gl->setSpacing(4);
    gl->setMargin(4);

    bAdded = true;
    readInTimes();
    changed = FALSE;
    QObject::connect( qApp, SIGNAL( clockChanged(bool) ),
                      this, SLOT( showTime() ) );
    // now start the timer so we can update the time quickly every second
    timerEvent( 0 );

    frmMap->setFocus();
    connect(frmMap, SIGNAL(signalTz(const QCString &)), this, SLOT(slotNewTz(const QCString &)));
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

    cfg.setGroup("TimeZones");

    int i;
    bool realTzWritten = FALSE;
    for ( i = 0;  i < CITIES; i++) {
	if ( !strCityTz[i].isNull() ) {
	    cfg.writeEntry("Zone"+QString::number(i), strCityTz[i]);
	    cfg.writeEntry("ZoneName"+QString::number(i), 
		    mMode == Minimal ? 
		    mCombo->text(i) :
		    listCities.at(i)->text());
	    if ( strCityTz[i] == strRealTz )
		realTzWritten = TRUE;
	}
    }
    if ( realTzWritten ) {
	cfg.removeEntry("Zone"+QString::number(CITIES));
	cfg.removeEntry("ZoneName"+QString::number(CITIES));
    } else {
	cfg.writeEntry("Zone"+QString::number(CITIES),
	    strRealTz);
	if ( nameRealTz.isEmpty() ) {
	    int i =  strRealTz.find( '/' );
	    nameRealTz = strRealTz.mid( i+1 );
	}
	cfg.writeEntry("ZoneName"+QString::number(CITIES),
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

void WorldTime::showTime( void )
{
    int i;
    QListIterator<CityInfo> itTime(listTimes);
    QDateTime curUtcTime = TimeZone::utcDateTime();

    // traverse the list...
    for ( i = 0, itTime.toFirst(); itTime.current(); i++, ++itTime) {
	itTime.current()->setUtcTime(curUtcTime);
    }
}


void WorldTime::beginNewTz()
{
    //const QObject *c = sender();
    //if ( c->inherits("QButton") && ((const QButton*)c)->isOn() ) {
	frmMap->setFocus();
	frmMap->beginEditing();
    //}
}

void WorldTime::slotNewTz( const QCString & zoneID)
{
    // determine what to do based on what button is pressed...
    TimeZone curZone;
    int i = 0;
    if (mMode == Minimal) {
	for (i = 0; i < CITIES; i++) {
	    if (strCityTz[i] == zoneID.data() && i != mCombo->currentItem()) {
		// city chose is already in the list.  Don't just abort, swap
		// them instead, as that is the most likely desired result from
		// the user.
		strCityTz[i] = strCityTz[mCombo->currentItem()];
		curZone = TimeZone( strCityTz[i] );
		mCombo->changeItem( curZone.city(), i);
		listTimes.at(i)->setZone(strCityTz[i]);
		break;
	    }
	}

	i = mCombo->currentItem();
	strCityTz[i] = zoneID;
	curZone = TimeZone( zoneID );
	mCombo->changeItem( curZone.city(), i);
	listTimes.at(i)->setZone(zoneID);
	changed = TRUE;
    } else {
	QListIterator<QPushButton> itCity(listCities);
	for ( ;itCity.current(); ++itCity) {
	    if ( strCityTz[i++] == zoneID.data() && !itCity.current()->isOn() )
		// should swap buttons instead of returning
		// (user may be trying to reorder buttons)
		return;
	}
    
    // go through the list and make adjustments based on which button is on
	for ( i = 0, itCity.toFirst(); itCity.current(); i++, ++itCity ) {
	    QPushButton *cmdTmp = itCity.current();
	    if ( cmdTmp->isOn() ) {
		strCityTz[i] = zoneID;
		curZone = TimeZone( zoneID );
                cmdTmp->setText( curZone.city() );
		listTimes.at(i)->setZone(zoneID);
		cmdTmp->toggle();
		// we can actually break, since there is only one button
		// that is ever pressed!
		changed = TRUE;
		break;
	    }
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

    int i;
    nameRealTz = QString::null;
    QString zn;
    nameRealTz = "";
    if (mCombo) mCombo->clear();
    for (i = 0; i < CITIES; i++ ) {
	zn = cfg.readEntry("Zone"+QString::number(i), QString::null);
	if ( zn.isNull() )
	    break;
	QString nm = cfg.readEntry("ZoneName"+QString::number(i));
	strCityTz[i] = zn;
	if (mMode == Minimal)
	    mCombo->insertItem( nm, i);
	else 
	    listCities.at(i)->setText(nm);
	listTimes.at(i)->setZone(zn);
	if ( zn == strRealTz )
	    nameRealTz = nm;
    }
    if ( nameRealTz.isEmpty() ) {
	//remember the current time zone even if we don't have room
	//to show it.
	zn = cfg.readEntry("Zone"+QString::number(CITIES),
	    QString::null);
	if ( zn == strRealTz )
	    nameRealTz = cfg.readEntry("ZoneName" +
		QString::number(CITIES));
	i++;
    }
}



