/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#include <QStackedWidget>
#include <QList>
#include <QTimerEvent>
#include <QPushButton>
#include <QToolButton>
#include <QGridLayout>

#include "cityinfo.h"
#include <qtopiaglobal.h>


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

class QTimeZone;
class QWorldmap;
class QStackedWidget;
class QComboBox;

class WORLDTIME_EXPORT WorldTime : public QWidget
{
    Q_OBJECT
public:
    WorldTime(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~WorldTime();

public slots:
    void beginNewTz();
    void beginNewTz(int);
    void addClock();
    void removeClock();
    void slotNewTz( const QTimeZone& zone );
    void slotComboSetZone( const int index );
    void slotNewTzCancelled();
    void saveChanges();
    void cancelChanges();
    void applyChange();

signals:
    void timeZoneListChange();

protected:
    void timerEvent( QTimerEvent* );
    void keyReleaseEvent( QKeyEvent * );
    
private slots:
    void showTime();
    void editMode();
    void viewMode();
    void selected();

private:
    void readInTimes( void );   // a method to get information from the config
    void writeTimezoneChanges();
    QString strRealTz;  // save the TZ var
    bool bAdded;        // a flag to indicate things have been added...
    int timerId;
    int currentCombo;
    int currentComboIndex;
    QString currentCity;
    int visibleZones;
    int maxVisibleZones;
    QGridLayout *gl;
    int getCurrentComboIndex(int);
    QAction *addClockAction;
    QAction *removeClockAction;
    void checkMenu();
    
    // a spot to hold the time zone for each city
    QString strCityTz[CITIES];
    QList<QPushButton *> listCities;
    QList<QComboBox *> listBoxes;    
    QList<CityInfo *> listTimes;
    QStackedWidget *mStack;
//    QComboBox *mCombo;
    bool changed;
    QToolButton *tb;
    QWorldmap *frmMap;
    enum SizeMode {
        Minimal,
        Tall,
        Wide
    } mMode;
};

#endif
