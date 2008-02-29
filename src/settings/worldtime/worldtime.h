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

#ifndef WORLDTIME_H
#define WORLDTIME_H
 
#include <qlist.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include "worldtimebase.h"
#include <qtopia/qpeglobal.h>


const int CITIES = 6;    // the number of cities...

#ifdef QTOPIA_DESKTOP
#ifndef Q_DEFINED_QLABEL
#define Q_DEFINED_QLABEL
#endif
#include <common/qpcwinexports.h>
#endif


class ZoneMap;

class QPC_EXPORT WorldTime : public WorldTimeBase
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
    void mousePressEvent( QMouseEvent* event );

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
    QList<QLabel> listTimes;
    bool changed;
};

#endif
