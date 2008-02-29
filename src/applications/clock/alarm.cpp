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

#include "alarm.h"

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qsettings.h>
#include <qtimestring.h>
#include <qtopianamespace.h>

#ifdef QTOPIA_PHONE
# include <qtopia/qsoftmenubar.h>
#endif

#include <qlabel.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qdesktopwidget.h>
#include <QKeyEvent>

static const int magic_daily = 2292922;
static const int magic_countdown = 2292923;

#if 0
static void toggleScreenSaver( bool on )
{
    QtopiaApplication::setTemporaryScreenSaverMode(on ? QtopiaApplication::Enable : QtopiaApplication::DisableSuspend);
}
#endif

class MySpinBox : public QSpinBox
{
public:
    QLineEdit *editor() const {
        return lineEdit();
    }
};

Alarm::Alarm( QWidget * parent, Qt::WFlags f )
    : QWidget( parent, f ), init(false) // No tr
{
    setupUi(this);
    alarmDlg = 0;
    alarmDlgLabel = 0;
    dayBtn = new QToolButton * [7];

    ampm = QTimeString::currentAMPM();
    onMonday = Qtopia::weekStartsOnMonday();

    applyAlarmTimer = new QTimer( this );
    applyAlarmTimer->setSingleShot(true);
    connect( applyAlarmTimer, SIGNAL(timeout()),
        this, SLOT(applyDailyAlarm()) );

    alarmt = new QTimer( this );
    connect( alarmt, SIGNAL(timeout()), SLOT(alarmTimeout()) );

    connect( qApp, SIGNAL(timeChanged()), SLOT(applyDailyAlarm()) );
    connect( qApp, SIGNAL(clockChanged(bool)), this, SLOT(changeClock(bool)) );

    connect( dailyHour, SIGNAL(valueChanged(int)), this, SLOT(scheduleApplyDailyAlarm()) );
    connect( dailyAmPm, SIGNAL(activated(int)), this, SLOT(setDailyAmPm(int)) );
    connect( dailyEnabled, SIGNAL(toggled(bool)), this, SLOT(enableDaily(bool)) );

    dailyMinute->installEventFilter( this );

    QSettings cConfig("Trolltech","Clock"); // No tr
    cConfig.beginGroup( "Daily Alarm" );

    QTimeString::Length len = QTimeString::Short;
    int approxSize = QFontMetrics(QFont()).width(" Wed ") * 7;
    QDesktopWidget *desktop = QApplication::desktop();
    if (desktop->availableGeometry(desktop->screenNumber(this)).width() > approxSize )
        len = QTimeString::Medium;

    int i;
    QVBoxLayout *vb = new QVBoxLayout( daysFrame );
    QHBoxLayout *weekdayBox = new QHBoxLayout();
    weekdayBox->setSpacing( 4 );
    weekdayBox->setMargin( 0 );
    weekdayBox->addStretch();
    QHBoxLayout *weekendBox = new QHBoxLayout();
    weekendBox->setSpacing( 4 );
    weekendBox->setMargin( 0 );
    weekendBox->addStretch();
    for ( i = 0; i < 7; i++ ) {
        dayBtn[i] = new QToolButton( daysFrame );
        if ( i < 5 )
            weekdayBox->addWidget( dayBtn[i] );
        else
            weekendBox->addWidget( dayBtn[i] );
        dayBtn[i]->setCheckable( true );
        dayBtn[i]->setChecked( true );
        dayBtn[i]->setFocusPolicy( Qt::StrongFocus );
        connect( dayBtn[i], SIGNAL(toggled(bool)), this, SLOT(scheduleApplyDailyAlarm()) );
    }

    weekdayBox->addStretch();
    weekendBox->addStretch();
    vb->addLayout( weekdayBox );
    vb->addLayout( weekendBox );

    for ( i = 0; i < 7; i++ )
        dayBtn[dayBtnIdx(i+1)]->setText( QTimeString::nameOfWeekDay(i + 1, len) );

    QStringList exclDays = cConfig.value( "ExcludeDays").toString().split( ',' );
    QStringList::Iterator it;
    for ( it = exclDays.begin(); it != exclDays.end(); ++it ) {
        int d = (*it).toInt();
        if ( d >= 1 && d <= 7 )
            dayBtn[dayBtnIdx(d)]->setChecked( false );
    }

    initEnabled = cConfig.value("Enabled", false).toBool();
    dailyEnabled->setChecked( initEnabled );
    int m = cConfig.value( "Minute", 0 ).toInt();
    dailyMinute->setValue( m );
    dailyMinute->setPrefix( m <= 9 ? "0" : "" );
    int h = cConfig.value( "Hour", 7 ).toInt();
    if ( ampm ) {
        if (h > 12) {
            h -= 12;
            dailyAmPm->setCurrentIndex( 1 );
        }
        if (h == 0) h = 12;
        dailyHour->setMinimum( 1 );
        dailyHour->setMaximum( 12 );
    } else {
        dailyAmPm->hide();
    }
    dailyHour->setValue( h );

    connect( ((MySpinBox*)dailyHour)->editor(), SIGNAL(textChanged(const QString&)),
            this, SLOT(dailyEdited()) );
    connect( ((MySpinBox*)dailyMinute)->editor(), SIGNAL(textChanged(const QString&)),
            this, SLOT(dailyEdited()) );

    init = true;
}

Alarm::~Alarm()
{
    applyDailyAlarm();
    delete [] dayBtn;
}

bool Alarm::isValid() const
{
    return !dailyEnabled->isChecked() || validDaysSelected();
}

void Alarm::changeClock( bool a )
{
    if ( ampm != a ) {
        int minute = dailyMinute->value();
        int hour = dailyHour->value();
        if ( ampm ) {
            if (hour == 12)
                hour = 0;
            if (dailyAmPm->currentIndex() == 1 )
                hour += 12;
            dailyHour->setMinimum( 0 );
            dailyHour->setMaximum( 23 );
            dailyAmPm->hide();
        } else {
            if (hour > 12) {
                hour -= 12;
                dailyAmPm->setCurrentIndex( 1 );
            }
            if (hour == 0) hour = 12;
            dailyHour->setMinimum( 1 );
            dailyHour->setMaximum( 12 );
            dailyAmPm->show();
        }
        dailyMinute->setValue( minute );
        dailyHour->setValue( hour );
    }
    ampm = a;
}

void Alarm::setDailyAmPm(int)
{
    scheduleApplyDailyAlarm();
}

bool Alarm::eventFilter( QObject* watched, QEvent* event )
{
    if ( watched == dailyMinute )
    {
        if ( event->type() == QEvent::FocusOut )
            dailyMinute->setPrefix( dailyMinute->value() <= 9 ? "0" : "" );
        else if ( event->type() == QEvent::FocusIn )
            dailyMinute->setPrefix( "" );
    }
    return false;
}

void Alarm::dailyEdited()
{
    if ( spinBoxValid(dailyMinute) && spinBoxValid(dailyHour) )
        scheduleApplyDailyAlarm();
    else
        applyAlarmTimer->stop();
}

void Alarm::enableDaily( bool )
{
    scheduleApplyDailyAlarm();
}

void Alarm::triggerAlarm(const QDateTime &when, int type)
{
    QTime theTime( when.time() );
    if ( type == magic_daily ) {
        QString msg = tr("<b>Daily Alarm:</b><p>");
        QString ts;
        if ( ampm ) {
            bool pm = false;
            int h = theTime.hour();
            if (h > 12) {
                h -= 12;
                pm = true;
            }
            if (h == 0) h = 12;
            ts.sprintf( "%02d:%02d %s", h, theTime.minute(), pm?"PM":"AM" );
        } else {
            ts.sprintf( "%02d:%02d", theTime.hour(), theTime.minute() );
        }
        msg += ts;
        Qtopia::soundAlarm();
        alarmCount = 0;
        alarmt->start( 5000 );
        if ( !alarmDlg ) {
            alarmDlg = new QDialog( this,
                    Qt::WindowStaysOnTopHint | Qt::WindowTitleHint );

            alarmDlg->setWindowTitle( tr("Clock") );
            QVBoxLayout *vb = new QVBoxLayout(alarmDlg);
            vb->setMargin(6);
            QHBoxLayout *hb = new QHBoxLayout( alarmDlg );
            vb->addLayout(hb);
            QLabel *l = new QLabel( alarmDlg );
            QPixmap pm(":image/alarmbell");
            l->setPixmap(pm);
            l->setFixedWidth(pm.width());
            hb->addWidget(l);
            alarmDlgLabel = new QLabel( msg, alarmDlg );
            alarmDlgLabel->setAlignment( Qt::AlignCenter );
            hb->addWidget(alarmDlgLabel);
#ifndef QTOPIA_PHONE
            QPushButton *cmdOk = new QPushButton( tr("OK"), alarmDlg );
            vb->addWidget(cmdOk);
            connect( cmdOk, SIGNAL(clicked()), alarmDlg, SLOT(accept()) );
#endif
        } else {
            alarmDlgLabel->setText(msg);
        }
        // Set for tomorrow, so user wakes up every day, even if they
        // don't confirm the dialog.
        applyDailyAlarm();
        if ( !alarmDlg->isVisible() ) {
            QtopiaApplication::execDialog(alarmDlg);
            alarmt->stop();
        }
    } else if ( type == magic_countdown ) {
        // countdown
        Qtopia::soundAlarm();
    }
}

void Alarm::setDailyEnabled(bool enableDaily)
{
    dailyEnabled->setChecked( enableDaily );
    applyDailyAlarm();
}

void Alarm::alarmTimeout()
{
    if ( alarmCount < 10 ) {
        Qtopia::soundAlarm();
        alarmCount++;
    } else {
        alarmCount = 0;
        alarmt->stop();
    }
}

QDateTime Alarm::nextAlarm( int h, int m )
{
    QDateTime now = QDateTime::currentDateTime();
    QTime at( h, m );
    QDateTime when( now.date(), at );
    int count = 0;
    int dow = when.date().dayOfWeek();
    while ( when < now || !dayBtn[dayBtnIdx(dow)]->isChecked() ) {
        when = when.addDays( 1 );
        dow = when.date().dayOfWeek();
        if ( ++count > 7 )
            return QDateTime();
    }

    return when;
}

int Alarm::dayBtnIdx( int d ) const
{
    if ( onMonday )
        return d-1;
    else if ( d == 7 )
        return 0;
    else
        return d;
}

void Alarm::scheduleApplyDailyAlarm()
{
    applyAlarmTimer->start( 5000 );
}

void Alarm::applyDailyAlarm()
{
    if ( !init )
        return;
    applyAlarmTimer->stop();
    int minute = dailyMinute->value();
    int hour = dailyHour->value();
    if ( ampm ) {
        if (hour == 12)
            hour = 0;
        if (dailyAmPm->currentIndex() == 1 )
            hour += 12;
    }

    QSettings config("Trolltech","Clock");
    config.beginGroup( "Daily Alarm" );
    config.setValue( "Hour", hour );
    config.setValue( "Minute", minute );

    bool enableDaily = dailyEnabled->isChecked();
    config.setValue( "Enabled", enableDaily );

    QString exclDays;
    int exclCount = 0;
    for ( int i = 1; i <= 7; i++ ) {
        if ( !dayBtn[dayBtnIdx(i)]->isChecked() ) {
            if ( !exclDays.isEmpty() )
                exclDays += ",";
            exclDays += QString::number( i );
            exclCount++;
        }
    }
    config.setValue( "ExcludeDays", exclDays );
    config.sync();

    if (enableDaily != initEnabled) {
        QtopiaIpcEnvelope e("QPE/AlarmServer", "dailyAlarmEnabled(bool)");
        e << enableDaily;
        initEnabled = enableDaily;
    }

    Qtopia::deleteAlarm(QDateTime(), "QPE/Application/clock",
            "alarm(QDateTime,int)", magic_daily);
    if ( dailyEnabled->isChecked() && exclCount < 7 ) {
        QDateTime when = nextAlarm( hour, minute );
        Qtopia::addAlarm(when, "QPE/Application/clock",
                            "alarm(QDateTime,int)", magic_daily);
    }
}

bool Alarm::validDaysSelected(void) const
{
    for ( int i = 1; i <= 7; i++ ) {
        if ( dayBtn[dayBtnIdx(i)]->isChecked() ) {
            return true;
        }
    }
    return false;
}

bool Alarm::spinBoxValid( QSpinBox *sb )
{
    bool valid = true;
    QString tv = sb->text();
    for ( int i = 0; i < tv.length(); i++ ) {
        if ( !tv[0].isDigit() )
            valid = false;
    }
    bool ok = false;
    int v = tv.toInt( &ok );
    if ( !ok )
        valid = false;
    if ( v < sb->minimum() || v > sb->maximum() )
        valid = false;

    return valid;
}

#ifdef QTOPIA_PHONE

void Alarm::keyPressEvent(QKeyEvent *ke)
{
    int i;
    switch(ke->key()) {
        case Qt::Key_Left:
            {
                // move back in days if possible
                // first find out which has focuse.
                QWidget *fw = focusWidget();

                for (i = 1; i < 7; i++) {
                    if (dayBtn[i] == fw) {
                        dayBtn[i-1]->setFocus();
                        break;
                    }
                }
                ke->accept();
                break;
            }
        case Qt::Key_Right:
            {
                // move forward in days if possible
                // first find out which has focuse.
                QWidget *fw = focusWidget();

                for (i = 0; i < 6; i++) {
                    if (dayBtn[i] == fw) {
                        dayBtn[i+1]->setFocus();
                        break;
                    }
                }
                ke->accept();
                break;
            }
    default:
        ke->ignore();
    }
}

#endif
