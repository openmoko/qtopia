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

#include "settime.h"

#include <qtopianamespace.h>
#include <qtopiaapplication.h>
#include <QTimeZoneSelector>
#include <qtimezone.h>
#include <qtimestring.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaipcadaptor.h>
#include <qsoftmenubar.h>
#include <custom.h>
#include <QTabWidget>
#include <QDateTimeEdit>
#include <QLabel>
#include <QLayout>
#include <QFormLayout>
#include <QSettings>
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>
#include <QStringListModel>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QValueSpace>
#include <QtopiaServiceRequest>

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../language/langname.h"


SetDateTime::SetDateTime(QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f ), tzEditable(true), tzLabel(0)
{
    setWindowTitle( tr("Date/Time") );

    QWidget *timePage, *formatPage;
    QFormLayout *timeLayout, *formatLayout;

    QTabWidget *tb = new QTabWidget(this);
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(tb);

    timePage = new QWidget;
    timeLayout = new QFormLayout;
    timePage->setLayout(timeLayout);

    formatPage = new QWidget;
    formatLayout = new QFormLayout;
    formatPage->setLayout(formatLayout);

    tb->addTab(timePage, tr("Time"));
    tb->addTab(formatPage, tr("Format"));

    atz = new QComboBox;
    atz->addItem(tr("Off"));
    atz->addItem(tr("Ask"));
    atz->addItem(tr("On"));
    timeLayout->addRow( tr("Automatic", "Time Automatic On/Ask/Off"), atz );

    tz = new QTimeZoneSelector;
    tz_label = new QLabel(tr( "Time Zone" ));
    tz_label->setBuddy(tz);
    tz_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    timeLayout->addRow(tz_label, tz);

    time = new QTimeEdit( QTime::currentTime() );
    time_label = new QLabel(tr("Time"));
    time_label->setBuddy(time);
    time_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    timeLayout->addRow(time_label, time);

    connect(time, SIGNAL(timeChanged(QTime)),
            this, SLOT(timeChange(QTime)) );

    date = new QDateEdit( QDate::currentDate() );
    date->setCalendarPopup( true );
    date_label = new QLabel(tr("Date"));
    date_label->setBuddy(date);
    date_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    timeLayout->addRow(date_label, date);

    connect(date, SIGNAL(dateChanged(QDate)),
            this, SLOT(dateChange(QDate)) );

    connect(atz, SIGNAL(activated(int)),
            this, SLOT(setAutomatic(int)));

    /* on to the format page/layout */

    ampmCombo = new QComboBox;
    ampmCombo->addItem( tr("24 hour") );
    ampmCombo->addItem( tr("12 hour") );
    formatLayout->addRow( tr("Time format"), ampmCombo );

    int show12hr = QTimeString::currentAMPM() ? 1 : 0;
    ampmCombo->setCurrentIndex( show12hr );
    updateTimeFormat( show12hr );

    connect(ampmCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateTimeFormat(int)));

    weekStartCombo = new QComboBox;
    weekStartCombo->addItem( tr("Sunday") );
    weekStartCombo->addItem( tr("Monday") );
    formatLayout->addRow( tr("Week starts" ), weekStartCombo );

    int startMonday = Qtopia::weekStartsOnMonday() ? 1 : 0;
    weekStartCombo->setCurrentIndex( startMonday );

    dateFormatCombo = new QComboBox;
    formatLayout->addRow( tr("Date format"), dateFormatCombo );

    QString df = QTimeString::currentFormat();
    date_formats = QTimeString::formatOptions();
    date_formats.prepend("loc");
    int currentdf = date_formats.indexOf(df);
    if (currentdf < 0)
        currentdf = 0;

    QStringList translated_date_formats;
    QString localename = languageName(Qtopia::languageList().first(), 0,0);
    translated_date_formats.append( localename );
    for (int i = 1; i< date_formats.count(); i++ ) {
        QString entry = date_formats[i];
        entry.replace( "D", tr("D", "D == day") );
        entry.replace( "M", tr("M", "M == month") );
        entry.replace( "Y", tr("Y", "Y == year") );
        translated_date_formats.append( entry );
    }
    connect(dateFormatCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateDateFormat()));

    QStringListModel *model = new QStringListModel(translated_date_formats, dateFormatCombo);
    dateFormatCombo->setModel(model);
    dateFormatCombo->setCurrentIndex( currentdf );

    connect(qApp, SIGNAL(timeChanged()),
            this, SLOT(sysTimeZoneChanged()) );

    QObject::connect( tz, SIGNAL(zoneChanged(QString)),
                      this, SLOT(tzChange(QString)) );

    timeChanged = false;
    dateChanged = false;
    tzChanged = false;

    //
    // Purge daily timer.  Avoids race between server triggering
    // daily alarm and the daily alarm getting removed then added
    // by the clock on receipt of timeChange().
    //
    Qtopia::deleteAlarm(QDateTime(), "QPE/Application/clock",
        "alarm(QDateTime,int)", -1);

    (void)QSoftMenuBar::menuFor( this );
    QSize ws = minimumSizeHint();
    // need to set size hint to more than 3/5's of screen
    // height else QtopiaApplication will not show it
    // maximized.
    QDesktopWidget *desktop = QApplication::desktop();
    QSize ds(desktop->availableGeometry(desktop->screenNumber(this)).size());
    if (ws.height() <= ds.height()*3/5) {
        ws.setHeight((ds.height()*3/5)+1);
        setMinimumSize(ws);
    }

    QSettings lconfig("Trolltech","locale");
    lconfig.beginGroup( "Location" );
    bool confatz = lconfig.value("TimezoneAuto").toBool();
    bool confatzp = lconfig.value("TimezoneAutoPrompt").toBool();

    // If it's automatic, but no good data received, turn automatic off
    bool autoworks = confatz && QDateTime::currentDateTime() > QDateTime(QDate(2007,11,20),QTime(0,0));
    atz->setCurrentIndex(autoworks ? confatzp ? 1 : 2 : 0);
    setAutomatic(autoworks);

    // XXX Location/TimeAuto and Location/TimeAutoPrompt ignored for now - no GUI

    new TimeService( this );
    new DateService( this );

    // timer to tick ahead time (and date if needed)
    clocktimer.start(1000,this);
}

void SetDateTime::setAutomatic(int mode)
{
    if (mode > 0 && sender()) { // ignore during initialization
        QtopiaServiceRequest req("TimeUpdate", "updateFromExternalSources(bool,bool,bool,bool)");
        req << true << true;
        req << (mode==1) << (mode==1);
        req.send();
    }

    tz->setEnabled(mode == 0);
    tz_label->setEnabled(mode == 0);
    time->setEnabled(mode == 0);
    time_label->setEnabled(mode == 0);
    date->setEnabled(mode == 0);
    date_label->setEnabled(mode == 0);
}

void SetDateTime::setTimezoneEditable(bool tze)
{
    if (tze == tzEditable)
        return;
    if (tze) {
        tz->show();
        delete tzLabel;
        tzLabel = 0;
    } else {
        tz->hide();
        QStringList tzNameParts = tz->currentZone().split('/');
        QString translatedTzName;
        for (QStringList::Iterator it = tzNameParts.begin(); it != tzNameParts.end(); ++it) {
            translatedTzName += qApp->translate("QTimeZone", (*it).toLatin1()); //no tr
            translatedTzName += '/';
        }
        if (!translatedTzName.isEmpty())
            translatedTzName = translatedTzName.left(translatedTzName.length()-1);
        tzLabel = new QLabel(translatedTzName, this);
    }
    tze = tzEditable;
}

void SetDateTime::editTime()
{
    time->setFocus();
    QtopiaApplication::instance()->showMainWidget();
}

void SetDateTime::editDate()
{
    date->setFocus();
    QtopiaApplication::instance()->showMainWidget();
}

QString SetDateTime::selectedDateFormat() const
{
    QString df;
    if ( dateFormatCombo->currentIndex() > 0 ) {
        df = date_formats[dateFormatCombo->currentIndex()];
        df.replace("D", "%D"); //convert to QTimeString format
        df.replace("M", "%M");
        df.replace("Y", "%Y");
    }
    return df;
}

void SetDateTime::storeSettings()
{
    // really turn off power saving before doing anything
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);

    // Need to process the QCOP event generated
    qApp->processEvents();

    bool monSunChange = false;

    {
        int startMonday =  Qtopia::weekStartsOnMonday();
        if ( startMonday != weekStartCombo->currentIndex() ) {
            Qtopia::setWeekStartsOnMonday(weekStartCombo->currentIndex() );
            monSunChange = true;
        }

        QSettings lconfig("Trolltech","locale");
        lconfig.beginGroup( "Location" );
        lconfig.setValue( "Timezone", tz->currentZone() );
        lconfig.setValue( "TimezoneAuto", atz->currentIndex() > 0 );
        lconfig.setValue( "TimeAuto", atz->currentIndex() > 0 ); // No GUI for this yet, copy as TimezoneAuto
        lconfig.setValue( "TimezoneAutoPrompt", atz->currentIndex() == 1 );
        lconfig.setValue( "TimeAutoPrompt", atz->currentIndex() == 1 ); // No GUI for this yet, copy as TimezoneAuto
    }

    if ( timeChanged || dateChanged || tzChanged ) {
        // before we progress further, set our TZ!
        setenv( "TZ", tz->currentZone().toLocal8Bit().constData(), 1 );

        QDateTime dt( date->date(), time->time() );
        if ( dt.isValid() ) {
            QtopiaServiceRequest req("TimeUpdate", "changeSystemTime(uint,QString)");
            req << (uint)dt.toTime_t() << tz->currentZone();
            req.send();
        } else {
            qWarning( "Invalid date/time" );
        }

    }

    // Notify everyone what day we prefer to start the week on.
    if ( monSunChange ) {
        QtopiaIpcEnvelope setWeek( "QPE/System", "weekChange(bool)" );
        setWeek << weekStartCombo->currentIndex();
    }

    bool dfch = false;
    {
        QSettings config("Trolltech","qpe");
        config.beginGroup( "Date" );
        QString df = selectedDateFormat();
        if ( df != config.value("DateFormat") ) {
            config.setValue("DateFormat", df);
            dfch = true;
        }
    }

    if ( dfch) // Notify everyone what date format to use
        QtopiaIpcEnvelope setDateFormat( "QPE/System", "setDateFormat()" );

    QSettings config("Trolltech","qpe");
    config.beginGroup( "Time" );
    int show12hr = config.value("AMPM").toBool() ? 1 : 0;
    bool ampm = ampmCombo->currentIndex()>0;
    if ( show12hr != ampm) {
        config.setValue( "AMPM", ampmCombo->currentIndex() );
        QtopiaIpcEnvelope setClock( "QPE/System", "clockChange(bool)" );
        setClock << (int)ampm;
    }

    // Restore screensaver
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
}

void SetDateTime::accept()
{
    storeSettings();
    QDialog::accept();
}

void SetDateTime::reject()
{
    QDialog::reject();
}

void SetDateTime::sysTimeZoneChanged()
{
    if ( !tzChanged || atz->currentIndex() != 0 ) {
        tz->setCurrentZone(::getenv("TZ"));
    }
}

void SetDateTime::tzChange( const QString &tz )
{
    QDateTime newDateTime = QTimeZone(tz.toLatin1())
            .convert( QDateTime::currentDateTime(), QTimeZone::current() );

    // Ignore this date change
    bool olddc = dateChanged;
    date->setDate( newDateTime.date() );
    dateChanged = olddc;

    // Ignore this time change
    bool oldtc = timeChanged;
    time->setTime( newDateTime.time() );
    timeChanged = oldtc;

    tzChanged = true;
}

void SetDateTime::dateChange( const QDate & )
{
    dateChanged = true;
}

void SetDateTime::timeChange( const QTime & )
{
    timeChanged = true;
}

void SetDateTime::timerEvent( QTimerEvent* )
{
    // Tick ahead. This re-assures the user.

    QDateTime now = QDateTime::currentDateTime();

    QString ctz = tz->currentZone();
    if ( ctz.isEmpty() )
        return; // (currently picking)
    QDateTime newDateTime = QTimeZone(ctz.toLatin1())
            .convert( now, QTimeZone::current() );
    if ( !timeChanged && !time->hasFocus() ) {
        time->setTime( newDateTime.time() );
        timeChanged = false; // ignore this change
    }
    if ( !dateChanged && !date->hasFocus() ) {
        date->setDate( newDateTime.date() );
        dateChanged = false; // ignore this change
    }

    clocktimer.start(qMax(500,(58-now.time().second())*1000),this);
}

void SetDateTime::updateDateFormat()
{
    QString df;
    if ( dateFormatCombo->currentIndex() > 0 ) {
        df = selectedDateFormat();
        df.replace(QString("%D"), QString("dd"));
        df.replace(QString("%M"), QString("MM"));
        df.replace(QString("%Y"), QString("yyyy"));
    } else {
        df = QLocale().dateFormat(QLocale::ShortFormat);
    }

    date->setDisplayFormat(df);
}

void SetDateTime::updateTimeFormat(int ampm)
{
    if (ampm)
        time->setDisplayFormat("h:mm ap");
    else
        time->setDisplayFormat("hh:mm");
}

/*!
    \service TimeService Time
    \brief Provides the Qtopia Time service.

    The \i Time service enables applications to provide a menu option
    or button that allows the user to edit the current system time
    without needing to implement an explicit time setting dialog.

    Client applications can request the \i Time service with the
    following code:

    \code
    QtopiaServiceRequest req( "Time", "editTime()" );
    req.send();
    \endcode

    \sa DateService, QtopiaAbstractService
*/

TimeService::TimeService( SetDateTime *parent )
    : QtopiaAbstractService( "Time", parent )
{
    this->parent = parent;
    publishAll();
}

/*!
    \internal
*/
TimeService::~TimeService()
{
}

/*!
    Instruct the \i Time service to display a dialog to allow the
    user to edit the current system time.

    This slot corresponds to the QCop service message \c{Time::editTime()}.
*/
void TimeService::editTime()
{
    parent->editTime();
}

/*!
    \service DateService Date
    \brief Provides the Qtopia Date service.

    The \i Date service enables applications to provide a menu option
    or button that allows the user to edit the current system date
    without needing to implement an explicit date setting dialog.

    Client applications can request the \i Date service with the
    following code:

    \code
    QtopiaServiceRequest req( "Date", "editDate()" );
    req.send();
    \endcode

    \sa TimeService, QtopiaAbstractService
*/
DateService::DateService( SetDateTime *parent )
    : QtopiaAbstractService( "Date", parent )
{
    this->parent = parent;
    publishAll();
}

/*!
    \internal
*/
DateService::~DateService()
{
}

/*!
    Instruct the \i Date service to display a dialog to allow the
    user to edit the current system date.

    This slot corresponds to the QCop service message \c{Date::editDate()}.
*/
void DateService::editDate()
{
    parent->editDate();
}
