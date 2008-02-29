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

#ifndef RECURRENCEDETAILS_H
#define RECURRENCEDETAILS_H

#include <qdialog.h>
#include <qdatetime.h>
#include "qtask.h"
#include "qappointment.h"

class QScrollArea;
class QLineEdit;
class QSpinBox;
class QGroupBox;
class QDateEdit;
class QComboBox;
class QCheckBox;
class QRadioButton;
class QLabel;
class QFormLayout;

class RecurrenceDetails : public QObject
{
    Q_OBJECT

public:
    RecurrenceDetails( QAppointment & appointment, QObject *parent = 0 );
    ~RecurrenceDetails();

    void initGui(QFormLayout *fl);

    bool eventFilter(QObject*, QEvent*);

public slots:
    void updateAppointment();
    void updateUI();

private slots:
    void repeatTypeChanged();
    void updateRepeatUntil();

private:
    void refreshLabels();
    QString trSmallOrdinal(int n) const;

private:
    bool mGuiInited;

    QAppointment& mAppointment;
    bool startMonday;
    QList<QAbstractButton *> weekDayToggle;
    QScrollArea *scrollArea;
    QComboBox *repeatCB;
    QDateEdit *untilDate;
    QWidget *repeatFieldsW;
    QLabel *repEveryL;
    QSpinBox *repEverySB;
    QGroupBox *monthlyGB, *weeklyGB, *untilGB;
    QRadioButton *dayOfMonth, *weekDayOfMonth, *lastWeekDayOfMonth;
};

#endif // RECURRENCEDIALOG_H
