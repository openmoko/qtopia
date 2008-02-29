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
#ifndef ENTRYDIALOG_H
#define ENTRYDIALOG_H

#ifdef QTOPIA_PHONE
#include "ui_entrydetails_phone.h"
#else
#include "ui_entrydetails.h"
#endif

#include <qtopia/pim/qappointment.h>

#include <QDateTime>
#include <QDialog>
#include <QScrollArea>

class QTextEdit;
class AppointmentDetails;
class QTabWidget;
class EntryDetails;
class QDLEditClient;

class EntryDialog : public QDialog
{
    Q_OBJECT

public:
    EntryDialog( bool startOnMonday, const QDateTime &start, const QDateTime &end,
                 QWidget *parent = 0, Qt::WFlags f = 0 );
    EntryDialog( bool startOnMonday, const QAppointment &appointment,
                 QWidget *parent = 0, Qt::WFlags f = 0 );
    ~EntryDialog();

    QAppointment appointment( const bool includeQdlLinks = true);

#ifdef QTOPIA_DESKTOP
    const EntryDetails *entryDetails() { return entry; }
#endif

signals:
    void categorymanagerChanged();

public slots:
    void updateStartDateTime();
    void updateStartTime();
    void updateEndDateTime();
    void updateEndTime();
    void editCustomRepeat();
    void setWeekStartsMonday( bool );
    void configureTimeZones();

    void turnOnAlarm();
    void checkAlarmSpin( int );

    void checkRepeatDate( bool );

    void updateCategories();
    void showSummary();

#ifndef QTOPIA_DESKTOP
protected:
    bool eventFilter( QObject *receiver, QEvent *event );
#endif

private slots:
    void setRepeatRule(int);
    void setEndDate(const QDate &);
    void tabChanged( QWidget *tab );

private:
    void init();
    void setDates( const QDateTime& s, const QDateTime& e );
    void setRepeatLabel();
    void accept();

#ifndef QTOPIA_DESKTOP
    QScrollArea *scrollArea;
#endif

    QAppointment mAppointment;
    QAppointment mOrigAppointment;
    bool startWeekOnMonday;
    EntryDetails *entry;
    QTextEdit *editNote;
    QDLEditClient *editnoteQC;
    AppointmentDetails *appointmentDetails;
    QTabWidget *tw;
};

// ====================================================================

class EntryDetails : public QWidget, public Ui::EntryDetailsBase
{
public:
    EntryDetails( QWidget *parent );
    virtual ~EntryDetails();
};

#endif // ENTRYDIALOG_H
