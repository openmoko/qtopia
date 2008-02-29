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
#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H

#include <qtopia/pim/qappointment.h>
#include <QDialog>

class QListView;
class QModelIndex;
class QOccurrenceModel;
class QStandardItemModel;
class QComboBox;
class QPushButton;
class QScrollArea;

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
    bool eventFilter( QObject *receiver, QEvent *event );

protected:
    void timerEvent(QTimerEvent *e);
    void keyPressEvent( QKeyEvent * ke);

private slots:
    void currentAlarmChanged(const QModelIndex& idx);
    void alarmSelected(const QModelIndex &idx);
    void snoozeClicked();

private:
    void formatDateTime(const QOccurrence& ev, bool useStartTime, QString& localDateTime, QString& realDateTime);

    Button mButton;

    int mAlarmCount;
    int mAlarmTimerId;
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
