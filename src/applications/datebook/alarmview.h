/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H

#include <qtopia/pim/qappointment.h>
#include <QDialog>
#include <QBasicTimer>

class QListView;
class QModelIndex;
class QOccurrenceModel;
class QStandardItemModel;
class QComboBox;
class QPushButton;
class QScrollArea;

class AlarmView : public QWidget
{
    Q_OBJECT
public:
    AlarmView( QWidget *parent, Qt::WFlags f = 0 );

    bool showAlarms(QOccurrenceModel *m, const QDateTime& startTime, int warnDelay);
    QOccurrence selectedOccurrence() const;

protected:
    void timerEvent(QTimerEvent *e);
    void keyPressEvent( QKeyEvent * ke);
    bool focusNextPrevChild(bool);

signals:
    void showAlarmDetails(const QOccurrence&);
    void closeView();

private slots:
    void currentAlarmChanged(const QModelIndex& idx);
    void alarmSelected(const QModelIndex &idx);
    void snoozeClicked();
    bool updateAlarms();

private:
    void init();
    void formatDateTime(const QOccurrence& ev, bool useStartTime, QString& localDateTime, QString& realDateTime);

    int mAlarmCount;
    QBasicTimer mAlarmTimer;

    QDateTime mStartTime;
    QOccurrenceModel* mModel;
    QStandardItemModel *mStandardModel;
    /* Widgetty stuff */
    QComboBox *mSnoozeChoices;
    QPushButton *mSnoozeButton;
    QScrollArea *scrollArea;
    QListView *mAlarmList;
};


#endif
