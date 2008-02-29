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

#ifndef REPEATENTRY_H
#define REPEATENTRY_H

#include "ui_repeatentry.h"

#include <qtimestring.h>
#include <qtopia/pim/qappointment.h>

#include <QDialog>
#include <QMap>

class QAppointment;

class RepeatEntry : public QDialog, Ui::RepeatEntryBase
{
    Q_OBJECT
public:
    RepeatEntry( QWidget *parent = 0 );
    ~RepeatEntry();

    void setAppointment(const QAppointment &);
    QAppointment appointment() const;

public slots:
    void  accept();

private slots:
    void refreshLabels();

private:
    QString trSmallOrdinal(int n) const;
    void setWeekDayToggles();
    void setAppointmentWeekFlags();

    QAppointment mAppointment;
    bool startMonday;
    QTimeString::Length weekLabelLen;
    QMap<Qt::DayOfWeek, QAbstractButton *> weekDayToggle;
};
#endif
