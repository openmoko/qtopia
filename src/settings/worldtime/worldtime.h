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

#ifndef WORLDTIME_H
#define WORLDTIME_H
 
#include <qlist.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <qvbox.h>
#include "cityinfo.h"
#include <qtopia/qpeglobal.h>


const int CITIES = 6;    // the number of cities...

#ifdef QTOPIA_DESKTOP
#ifndef Q_DEFINED_QLABEL
#define Q_DEFINED_QLABEL
#endif
#ifdef Q_OS_WIN32
#include <common/qpcwinexports.h>
#endif
#define WORLDTIME_EXPORT QPC_EXPORT
#else
#define WORLDTIME_EXPORT
#endif


class ZoneMap;
class QWidgetStack;
class QComboBox;

class WORLDTIME_EXPORT WorldTime : public QWidget
{
    Q_OBJECT
public:
    WorldTime(QWidget* parent = 0, const char *name = 0, WFlags fl = 0);
    ~WorldTime();

public slots:
    void beginNewTz();
    void slotNewTz( const QCString& zoneID);
    void saveChanges();
    void cancelChanges();

signals:
    void timeZoneListChange(); 

protected:
    void timerEvent( QTimerEvent* );

private slots:
    void showTime();

private:
    void readInTimes( void );   // a method to get information from the config
    void writeTimezoneChanges();
    QString strRealTz;  // save the TZ var
    QString nameRealTz; // and what it is called
    bool bAdded;        // a flag to indicate things have been added...
    int timerId;
    
    // a spot to hold the time zone for each city
    QString strCityTz[CITIES];
    QList<QPushButton> listCities;
    QList<CityInfo> listTimes;
    QWidgetStack *mStack;
    QComboBox *mCombo;
    bool changed;
    ZoneMap *frmMap;
    enum SizeMode {
	Minimal,
	Tall,
	Wide
    } mMode; 
};

#endif
