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
#ifndef ENTRYDIALOG_H
#define ENTRYDIALOG_H

#ifdef QTOPIA_PHONE
#include "entrydetails_phone.h"
#else
#include "entrydetails.h"
#endif

#include <qtopia/pim/event.h>

#include <qdatetime.h>
#include <qdialog.h>

class QMultiLineEdit;
class EventView;
class QTabWidget;

class EntryDialog : public QDialog
{
    Q_OBJECT

public:
    EntryDialog( bool startOnMonday, const QDateTime &start, const QDateTime &end,
	         QWidget *parent = 0, const char *name = 0, bool modal = FALSE, WFlags f = 0 );
    EntryDialog( bool startOnMonday, const QDate &occurrence, const PimEvent &event,
	         QWidget *parent = 0, const char *name = 0, bool modal = FALSE, WFlags f = 0 );
    ~EntryDialog();

    PimEvent event();

#ifdef QTOPIA_DESKTOP
    const EntryDetails *entryDetails() { return entry; }
#endif

signals:
    void categoriesChanged();

public slots:
    void endDateTimeChanged( const QDateTime & );
    void startDateTimeChanged( const QDateTime & );
    void slotRepeat();
    void setWeekStartsMonday( bool );
    void allDayToggled( bool );
    void configureTimeZones();

    void turnOnAlarm();
    void checkAlarmSpin( int );

    void updateCategories();
    void showSummary();

private slots:
    void setRepeatType(int);
    void setEndDate(const QDate &);
    void tabChanged( QWidget *tab );

private:
    void init();
    void setDates( const QDateTime& s, const QDateTime& e );
    void setRepeatLabel();
    void accept();

    QDate mOccurrence;
    PimEvent mEvent;
    PimEvent mOrigEvent;
    bool startWeekOnMonday;
    EntryDetails *entry;
    QMultiLineEdit *editNote;
    EventView *eventView;
    QTabWidget *tw;
};


#endif // ENTRYDIALOG_H
