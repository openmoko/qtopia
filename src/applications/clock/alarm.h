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
#ifndef ALARM_H
#define ALARM_H

#include "ui_alarmbase.h"
#include <qdatetime.h>
#include <QHash>

class QTimer;
class QLabel;
class QDialog;
class QEvent;

class Alarm : public QWidget, Ui::AlarmBase
{
    Q_OBJECT
public:
    Alarm( QWidget *parent=0, Qt::WFlags fl=0 );
    ~Alarm();

    void triggerAlarm(const QDateTime &when, int type);
    bool eventFilter(QObject *o, QEvent *e);
    void setRingPriority(bool);
public slots:
    void setDailyEnabled(bool);

private slots:
    void changeClock( bool );
    void alarmTimeout();
    void applyDailyAlarm();
    void changeAlarmDays();

protected:
    QDateTime nextAlarm( int h, int m );
    QString getAlarmDaysText() const;
    void resetAlarmDaysText();

private:
    QTimer *alarmt;
    bool ampm;
    bool weekStartsMonday;
    int alarmCount;
    bool initEnabled;
    QDialog* alarmDlg;
    QLabel* alarmDlgLabel;
    bool init;
    QHash<int, bool> daysSettings;
};

#endif

