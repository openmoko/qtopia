/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
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

#ifdef QTOPIA_PHONE
# include <qtopia/contextmenu.h>
#endif

class QToolButton;
class QSpinBox;
class QLabel;
class QHBoxLayout;
class TimeZoneSelector;
class QComboBox;
class QPEDialogListener;

class SetTime : public QWidget
{
    Q_OBJECT
public:
    SetTime( QWidget *parent=0, const char *name=0 );

    QTime time() const;

    bool changed() const { return userChanged; }

public slots:
    void slotTzChange( const QString& tz );
    void show12hourTime( int );

protected slots:
    void hourChanged( int value );
    void minuteChanged( int value );

    void checkedPM( int );

protected:
    void focusInEvent( QFocusEvent *e );

    int hour;
    int minute;
    bool use12hourTime;
    QComboBox *ampm;
    QSpinBox *sbHour;
    QSpinBox *sbMin;
    bool userChanged;
};

class QPEDateEdit;

class SetDateTime : public QDialog
{
    Q_OBJECT
public:
    SetDateTime( QWidget *parent=0, const char *name=0, bool modal=FALSE,  WFlags f=0 );

    void setTimezoneEditable(bool tze);

protected slots:
    void appMessage(const QCString&, const QByteArray&);
    void tzChange( const QString &tz );
    void formatChanged(int);
    void weekStartChanged(int);
    void dateChange(const QDate &);

protected:
    virtual void accept();
    virtual void done(int);

    QWidget *timePage;
    SetTime *time;
    QPEDateEdit *date;
    TimeZoneSelector *tz;
    QComboBox *weekStartCombo;
    QComboBox *ampmCombo;
    QComboBox *dateFormatCombo;

    QPEDialogListener *dl;

    QArray<DateFormat> date_formats;
    bool dateChanged;
    bool tzChanged;
    bool tzEditable;
    QLabel *tzLabel;
    QHBoxLayout *tzLayout;
#ifdef QTOPIA_PHONE
    ContextMenu *contextMenu;
#endif    
};


#endif

