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
#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H


#include <qdatetime.h>
#include <qdialog.h>

#include <qtopia/timestring.h>

class QToolButton;
class QSpinBox;
class QLabel;
class TimeZoneSelector;
class QComboBox;
class QPEDialogListener;

class SetTime : public QWidget
{
    Q_OBJECT
public:
    SetTime( QWidget *parent=0, const char *name=0 );

    QTime time() const;

public slots:
    void slotTzChange( const QString& tz );
    void show12hourTime( int );

protected slots:
    void hourChanged( int value );
    void minuteChanged( int value );

    void checkedPM( int );

protected:
    int hour;
    int minute;
    bool use12hourTime;
    QComboBox *ampm;
    QSpinBox *sbHour;
    QSpinBox *sbMin;
};

class QPEDateEdit;

class SetDateTime : public QDialog
{
    Q_OBJECT
public:
    SetDateTime( QWidget *parent=0, const char *name=0, bool modal=FALSE,  WFlags f=0 );

protected slots:
    void tzChange( const QString &tz );
    void formatChanged(int);
    void weekStartChanged(int);

protected:
    virtual void accept();
    virtual void done(int);

    SetTime *time;
    QPEDateEdit *date;
    TimeZoneSelector *tz;
    QComboBox *weekStartCombo;
    QComboBox *ampmCombo;
    QComboBox *dateFormatCombo;

    QPEDialogListener *dl;

    DateFormat date_formats[4];
};


#endif

