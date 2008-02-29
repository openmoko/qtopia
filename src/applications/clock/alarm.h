/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef ALARM_H
#define ALARM_H

#include "ui_alarmbase.h"
#include <qdatetime.h>

class QTimer;
class QLabel;
class QDialog;
class QToolButton;

class Alarm : public QWidget, Ui::AlarmBase
{
    Q_OBJECT
public:
    Alarm( QWidget *parent=0, Qt::WFlags fl=0 );
    ~Alarm();

    void setDailyEnabled(bool);
    void triggerAlarm(const QDateTime &when, int type);
    bool isValid() const;

    bool eventFilter( QObject* watched, QEvent* event );

private slots:
    void changeClock( bool );
    void setDailyAmPm( int );
    void dailyEdited();
    void enableDaily( bool );
    void alarmTimeout();
    void applyDailyAlarm();
    void scheduleApplyDailyAlarm();

protected:
    QDateTime nextAlarm( int h, int m );
    int dayBtnIdx( int ) const;
    bool spinBoxValid( QSpinBox *sb );
    bool validDaysSelected(void) const;

#ifdef QTOPIA_PHONE
    void keyPressEvent(QKeyEvent *);
#endif

private:
    QTimer *alarmt;
    bool ampm;
    bool onMonday;
    int alarmCount;
    bool initEnabled;
    QDialog* alarmDlg;
    QLabel* alarmDlgLabel;
    QToolButton **dayBtn;
    bool init;
    QTimer *applyAlarmTimer;
};

#endif

