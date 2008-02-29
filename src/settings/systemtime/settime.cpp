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

#include "settime.h"

#include <qtopianamespace.h>
#include <qtopiaapplication.h>

#include <qtimezonewidget.h>
#include <qtimezone.h>
#include <qtimestring.h>
#include <stdlib.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaipcadaptor.h>
#include <custom.h>
#ifdef QTOPIA_PHONE
#include <QTabWidget>
#endif
#include <QDateTimeEdit>
#include <QLabel>
#include <QLayout>
#include <QSettings>
#include <QSpinBox>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include <QComboBox>
#include <QDebug>
#include <QStringListModel>
#include <QCloseEvent>
#include <QDesktopWidget>

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

SetDateTime::SetDateTime(QWidget *parent, Qt::WFlags f )
    : QMainWindow( parent, f ), tzEditable(true), tzLabel(0)
{
    setWindowTitle( tr("Date/Time") );

    QWidget *timePage, *formatPage;
    QVBoxLayout *timeLayout, *formatLayout;
#ifdef QTOPIA_PHONE
    QTabWidget *tb = new QTabWidget;
    QVBoxLayout *vb = new QVBoxLayout;
    vb->setMargin(0);
    vb->addWidget(tb);
    setLayout(vb);

    timePage = new QWidget;
    timeLayout = new QVBoxLayout;
    timeLayout->setMargin(0);
    timePage->setLayout(timeLayout);

    formatPage = new QWidget;
    formatLayout = new QVBoxLayout;
    formatLayout->setMargin(0);
    formatPage->setLayout(formatLayout);

    tb->addTab(timePage, tr("Time"));
    tb->addTab(formatPage, tr("Format"));

    setCentralWidget( tb );
#else
    timePage = formatPage = new QWidget( this );
    timeLayout = formatLayout = new QVBoxLayout;
    timePage->setLayout(timeLayout);
    setCentralWidget( timePage );
#endif

    tzLayout = new QHBoxLayout;
    timeLayout->addLayout(tzLayout);

    QLabel *lblZone = new QLabel(
#ifdef QTOPIA_PHONE
        tr( "T.Z." )
#else
        tr( "Time Zone" )
#endif
        );

    lblZone->setMaximumSize( lblZone->sizeHint() );
    tzLayout->addWidget( lblZone );

    tz = new QTimeZoneWidget;
    tzLayout->addWidget( tz );

    time = new SetTime;
    timeLayout->addWidget( time );

    QHBoxLayout *db = new QHBoxLayout;
    timeLayout->addLayout(db);
    QLabel *dateLabel = new QLabel( tr("Date"));
    db->addWidget( dateLabel, 1 );

    /* reparented on layout, but still, need better constructor */
    date = new QDateEdit( QDate::currentDate() );

    connect(date, SIGNAL(dateChanged(const QDate&)),
            this, SLOT(dateChange(const QDate&)) );
    db->addWidget( date, 2 );

#ifndef QTOPIA_PHONE
    QDesktopWidget *desktop = QApplication::desktop();
    if (desktop->screenGeometry(desktop->screenNumber(this)).height() >= 220) {
        QFrame *hline = new QFrame;
        hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );
        timeLayout->addWidget( hline );
    }
#endif

    QSettings config("Trolltech","qpe");
    config.beginGroup( "Time" );

    /* on to the format page/layout */
    QHBoxLayout *hb1 = new QHBoxLayout;
    formatLayout->addLayout(hb1);

    QLabel *l = new QLabel( tr("Time format") );
    //    l->setAlignment( AlignRight | AlignVCenter );
    hb1->addWidget( l, 1 );


    ampmCombo = new QComboBox;
    ampmCombo->addItem( tr("24 hour") );
    ampmCombo->addItem( tr("12 hour") );
    hb1->addWidget( ampmCombo, 2 );

    int show12hr = QTimeString::currentAMPM() ? 1 : 0;
    ampmCombo->setCurrentIndex( show12hr );
    time->show12hourTime( show12hr );

    connect(ampmCombo, SIGNAL(activated(int)),
            time, SLOT(show12hourTime(int)));

    QHBoxLayout *hb2 = new QHBoxLayout;
    formatLayout->addLayout(hb2);
    l = new QLabel( tr("Week starts" ) );
    //l->setAlignment( AlignRight | AlignVCenter );
    hb2->addWidget( l, 1 );

    weekStartCombo = new QComboBox;
    weekStartCombo->addItem( tr("Sunday") );
    weekStartCombo->addItem( tr("Monday") );

    hb2->addWidget( weekStartCombo, 2 );
    int startMonday = Qtopia::weekStartsOnMonday() ? 1 : 0;
    weekStartCombo->setCurrentIndex( startMonday );

    QHBoxLayout *hb3 = new QHBoxLayout;
    formatLayout->addLayout(hb3);
    l = new QLabel( tr("Date format" ) );
    hb3->addWidget( l, 1 );
    dateFormatCombo = new QComboBox;
    hb3->addWidget( dateFormatCombo, 2 );

    QString df = QTimeString::currentFormat();
    date_formats = QTimeString::formatOptions();
    date_formats.prepend("loc");
    int currentdf = date_formats.indexOf(df);
    if (currentdf < 0)
        currentdf = 0;

    QStringList translated_date_formats;
    translated_date_formats.append( tr("locale", "Use the date format for the current language") );
    for (int i = 1; i< date_formats.count(); i++ ) {
        QString entry = date_formats[i];
        entry.replace( "D", tr("D", "D == day") );
        entry.replace( "M", tr("M", "M == month") );
        entry.replace( "Y", tr("Y", "Y == year") );
        translated_date_formats.append( entry );
    }

    QStringListModel *model = new QStringListModel(translated_date_formats, dateFormatCombo);
    dateFormatCombo->setModel(model);
    dateFormatCombo->setCurrentIndex( currentdf );

    connect(dateFormatCombo, SIGNAL(activated(int)),
            this, SLOT(setDateFormat()));
    connect(qApp, SIGNAL(dateFormatChanged()),
            this, SLOT(updateDateFormat()));

#ifdef QTOPIA_PHONE
    formatLayout->addStretch( 0 );
#endif
    timeLayout->addStretch( 0 );

    QObject::connect( tz, SIGNAL( signalNewTz(const QString&) ),
                      time, SLOT( slotTzChange(const QString&) ) );
    QObject::connect( tz, SIGNAL( signalNewTz(const QString&) ),
                      this, SLOT( tzChange(const QString&) ) );

    dateChanged = false;
    tzChanged = false;

    //
    // Purge daily timer.  Avoids race between server triggering
    // daily alarm and the daily alarm getting removed then added
    // by the clock on receipt of timeChange().
    //
    Qtopia::deleteAlarm(QDateTime(), "QPE/Application/clock",
        "alarm(QDateTime,int)", -1);

#ifdef QTOPIA_PHONE
    contextMenu = QSoftMenuBar::menuFor( this );
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
#endif
    new TimeService( this );
    new DateService( this );
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
        tzLayout->addWidget(tzLabel);
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

void SetDateTime::storeSettings()
{
    tz->setFocus();

    // really turn off power saving before doing anything
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);

    // Need to process the QCOP event generated
    qApp->processEvents();

    bool ampmChange = false;
    bool monSunChange = false;

    {
        QSettings config("Trolltech","qpe");
        config.beginGroup( "Time" );
        int show12hr = config.value("AMPM").toBool() ? 1 : 0;
        if ( show12hr != ampmCombo->currentIndex() ) {
            config.setValue( "AMPM", ampmCombo->currentIndex() );
            ampmChange = true;
        }
        int startMonday =  Qtopia::weekStartsOnMonday();
        if ( startMonday != weekStartCombo->currentIndex() ) {
            Qtopia::setWeekStartsOnMonday(weekStartCombo->currentIndex() );
            monSunChange = true;
        }

        QSettings lconfig("Trolltech","locale");
        lconfig.beginGroup( "Location" );
        lconfig.setValue( "Timezone", tz->currentZone() );
    }

    if ( time->changed() || dateChanged || tzChanged ) {
        // before we progress further, set our TZ!
        setenv( "TZ", tz->currentZone().toLocal8Bit().constData(), 1 );

#if ( defined Q_WS_QWS )
#if defined(QTOPIA_ZONEINFO_PATH)
        QString     filename = QTOPIA_ZONEINFO_PATH + tz->currentZone();
#else
        QString     filename = "/usr/share/zoneinfo/" + tz->currentZone();
#endif
        QString cmd = "cp -f " + filename + " /etc/localtime";
        system(cmd.toLocal8Bit().constData());
#endif
        // now set the time...
        QDateTime dt( date->date(), time->time() );

        if ( dt.isValid() ) {
            struct timeval myTv;
            myTv.tv_sec = dt.toTime_t();
            myTv.tv_usec = 0;

            if ( myTv.tv_sec != -1 )
                ::settimeofday( &myTv, 0 );
            Qtopia::writeHWClock();
            // Should leave updating alarms to datebook, rather than screw it up
            // via duplicated functionality.
            // DateBookDB is flawed, it should not be used anywhere.
        } else {
            qWarning( "Invalid date/time" );
        }
        // set the timezone for everyone else...
        QtopiaIpcEnvelope setTimeZone( "QPE/System", "timeChange(QString)" );
        setTimeZone << tz->currentZone();
    }

    // AM/PM setting and notify time changed
    if ( ampmChange ) {
        QtopiaIpcEnvelope setClock( "QPE/System", "clockChange(bool)" );
        setClock << ampmCombo->currentIndex();
    }

    // Notify everyone what day we prefer to start the week on.
    if ( monSunChange ) {
        QtopiaIpcEnvelope setWeek( "QPE/System", "weekChange(bool)" );
        setWeek << weekStartCombo->currentIndex();
    }

    // Restore screensaver
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);

    //QDialog::accept();
}

void SetDateTime::closeEvent( QCloseEvent* e )
{
    storeSettings();
    e->accept();
}

void SetDateTime::tzChange( const QString &tz )
{
    QDateTime newDate = QTimeZone(tz.toLatin1()).convert( QDateTime::currentDateTime(),
            QTimeZone::current() );
    date->setDate( newDate.date() );
    tzChanged = true;
}

void SetDateTime::dateChange( const QDate & )
{
    dateChanged = true;
}

void SetDateTime::setDateFormat()
{
    {
        QSettings config("Trolltech","qpe");
        config.beginGroup( "Date" );
        QString df;
        if ( dateFormatCombo->currentIndex() > 0 ) {
            df = date_formats[dateFormatCombo->currentIndex()];
            df.replace("D", "%D"); //convert to QTimeString format
            df.replace("M", "%M");
            df.replace("Y", "%Y");
        }
        config.setValue("DateFormat", df);
    }

    // Notify everyone what date format to use
    QtopiaIpcEnvelope setDateFormat( "QPE/System", "setDateFormat()" );
}

void SetDateTime::updateDateFormat()
{
    date->setFocus();
    dateFormatCombo->setFocus();
}

// ============================================================================
//
// MinuteSpinBox
//
// ============================================================================

MinuteSpinBox::MinuteSpinBox( QWidget* parent )
:   QSpinBox( parent )
{
}

QString MinuteSpinBox::textFromValue( int value ) const
{
    if ( value < 10 )
        return "0" + QString::number( value );
    else
        return QString::number( value );
}

// ============================================================================
//
// SetTime
//
// ============================================================================

static const int ValueAM = 0;
static const int ValuePM = 1;

SetTime::SetTime( QWidget *parent )
    : QWidget( parent )
{
    use12hourTime = false;

    QTime currTime = QTime::currentTime();
    hour = currTime.hour();
    minute = currTime.minute();

    QHBoxLayout *hb2 = new QHBoxLayout( this );
    hb2->setMargin( 0 );

    QLabel *l = new QLabel( tr("Time"), this );
    //    l->setAlignment( AlignRight | AlignVCenter );
    hb2->addWidget( l );

    sbHour = new QSpinBox( this );
    sbHour->setMinimumWidth( 30 );
    if(use12hourTime) {
        sbHour->setRange(1, 12);
        int show_hour = hour;
        if (hour > 12)
            show_hour -= 12;
        if (show_hour == 0)
            show_hour = 12;

        sbHour->setValue( show_hour );
    } else {
        sbHour->setRange( 0, 23 );
        sbHour->setValue( hour );
    }
    sbHour->setWrapping(true);
    connect( sbHour, SIGNAL(valueChanged(int)), this, SLOT(hourChanged(int)) );
    hb2->addWidget( sbHour );

    hb2->addStretch( 1 );

    l = new QLabel( tr(":"), this );
    //l->setAlignment( AlignRight | AlignVCenter );
    hb2->addWidget( l );

    sbMin = new MinuteSpinBox( this );
    sbMin->setRange( 0,59 );
    sbMin->setWrapping(true);
    sbMin->setValue( minute );
    minuteChanged(minute);
    sbMin->setMinimumWidth( 30 );
    connect( sbMin, SIGNAL(valueChanged(int)), this, SLOT(minuteChanged(int)) );
    hb2->addWidget( sbMin );

    hb2->addStretch( 1 );

    ampm = new QComboBox( this );
    ampm->addItem( tr("AM") );
    ampm->addItem( tr("PM") );
    connect( ampm, SIGNAL(activated(int)), this, SLOT(checkedPM(int)) );
    hb2->addWidget( ampm );

    hb2->addStretch( 1 );

    userChanged = false;
}

QTime SetTime::time() const
{
    return QTime( hour, minute, 0 );
}

void SetTime::focusInEvent( QFocusEvent *e )
{
    QWidget::focusInEvent( e );
    //sbHour->setFocus();   //HACK: not needed, and causes problems with edit focus display
#ifdef QTOPIA_PHONE
    if( !Qtopia::mousePreferred() ) {
        if( sbHour->hasEditFocus() )
            sbHour->setEditFocus( true );
    }
#endif
}

void SetTime::hourChanged( int value )
{
    if(use12hourTime) {
        int realhour = value;
        if (realhour == 12)
            realhour = 0;
        if (ampm->currentIndex() == ValuePM )
            realhour += 12;
        hour = realhour;
    } else
        hour = value;

    userChanged = true;
}

void SetTime::minuteChanged( int value )
{
    minute = value;
    userChanged = true;
}

void SetTime::show12hourTime( int on )
{
    bool uc = userChanged;
    use12hourTime = on;
    ampm->setEnabled(on);

    int show_hour = hour;
    if ( on ) {
        /* this might change the value of hour */
        sbHour->setRange(1, 12);

        /* so use one we saved earlier */
        if (show_hour >= 12) {
            show_hour -= 12;
            ampm->setCurrentIndex( ValuePM );
        } else {
            ampm->setCurrentIndex( ValueAM );
        }
        if (show_hour == 0)
            show_hour = 12;

    } else {
        sbHour->setRange( 0, 23 );
    }

    sbHour->setValue( show_hour );
    userChanged = uc;
}

void SetTime::checkedPM( int c )
{
    int show_hour = sbHour->value();
    if (show_hour == 12)
        show_hour = 0;

    if ( c == ValuePM )
        show_hour += 12;

    hour = show_hour;
    userChanged = true;
}

void SetTime::slotTzChange( const QString &tz )
{
    QTime newTime = QTimeZone( tz.toLatin1() )
        .convert( QDateTime::currentDateTime(), QTimeZone::current() ).time();

    // just set the spinboxes and let it propagate through
    if(use12hourTime) {
        int show_hour = newTime.hour();
        if (newTime.hour() >= 12) {
            show_hour -= 12;
            ampm->setCurrentIndex( ValuePM );
        } else {
            ampm->setCurrentIndex( ValueAM );
        }
        if (show_hour == 0)
            show_hour = 12;
        sbHour->setValue( show_hour );
    } else {
        sbHour->setValue( newTime.hour() );
    }
    sbMin->setValue( newTime.minute() );
    userChanged = true;
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
