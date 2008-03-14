/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef WORLDTIME_H
#define WORLDTIME_H

// Qt4 Headers
#include <QTimerEvent>
#include <QPushButton>
#include <QGridLayout>

#include "cityinfo.h"
#include <qtopiaglobal.h>


const int CITIES = 6;    // the number of cities...

#define WORLDTIME_EXPORT

class QTimeZone;
class QWorldmap;
class QComboBox;

class WORLDTIME_EXPORT WorldTime : public QWidget
{
    Q_OBJECT
public:
    WorldTime(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~WorldTime();

public slots:
    void beginNewTz();
    void slotNewTz( const QTimeZone& zone );
    void slotSetZone();
    void slotNewTzCancelled();
    void saveChanges();
    void cancelChanges();
 
signals:
    void timeZoneListChange();

protected:
    bool isEditMode;
    void timerEvent( QTimerEvent* );void keyReleaseEvent( QKeyEvent * );


private slots:
    void showTime();
    void editMode();
    void viewMode();
    void selected();

private:
    int isHighlighted;
    void readInTimes( void );   // a method to get information from the config
    void writeTimezoneChanges();
    int findCurrentButton();
    void setButtonAvailable(int selButton);
   
    int timerId;
    int maxVisibleZones;
    QGridLayout *gl;

    // a spot to hold the time zone for each city
    QString strCityTz[CITIES];
    QList<QPushButton *> listCities;
    QPushButton *currentPushButton;
    QList<CityInfo *> listTimes;
    bool changed;

    QWorldmap *frmMap;
    enum SizeMode {
        Minimal,
        Medium,
        Tall,
        Wide
    } mMode;
};

#endif
