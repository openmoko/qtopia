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
#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H

#include <qtopia/pim/qappointment.h>
#include <QDialog>

class QListView;
class QModelIndex;
class QOccurrenceModel;
class AlarmFilterModel;

class AlarmDialog : public QDialog
{
    Q_OBJECT
public:
    AlarmDialog( QWidget *parent, Qt::WFlags f = 0 );

    void init();

    enum Button { Cancel = 0, Snooze, Details };
    Button exec(QOccurrenceModel *m, const QDateTime& startTime, int warnDelay);
    Button result();
    QOccurrence selectedOccurrence() const;

protected:
    void timerEvent(QTimerEvent *e);
    void keyPressEvent( QKeyEvent * ke);

private slots:
    void currentAlarmChanged(const QModelIndex& idx);
    void alarmSelected(const QModelIndex &idx);

private:
    Button mButton;

    int mAlarmCount;
    int mAlarmTimerId;
    QOccurrenceModel* mModel;
    AlarmFilterModel *mFilterModel;

    /* Widgetty stuff */
    QListView *mAlarmList;
};


#endif
