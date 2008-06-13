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

#include "alarm.h"

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qsettings.h>
#include <qtimestring.h>
#include <qtopianamespace.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtimer.h>
#include <QDateTimeEdit>
#include <QSoundControl>

#ifdef Q_WS_X11
#include <qcopchannel_x11.h>
#else
#include <qcopchannel_qws.h>
#endif

static const int magic_daily = 2292922;     //type for daily alarm

void Alarm::setRingPriority(bool v)
{
    QtopiaIpcEnvelope   e("QPE/MediaServer", "setPriority(int)");
    e << ((v)?QSoundControl::RingTone : QSoundControl::Default);
} 



Alarm::Alarm( QWidget * parent, Qt::WFlags f )
    : QWidget( parent, f ), init(false) // No tr
{
    setupUi(this);
    alarmDlg = 0;
    alarmDlgLabel = 0;

    ampm = QTimeString::currentAMPM();
    weekStartsMonday = Qtopia::weekStartsOnMonday();

    alarmt = new QTimer( this );
    connect( alarmt, SIGNAL(timeout()), SLOT(alarmTimeout()) );

    connect( qApp, SIGNAL(timeChanged()), SLOT(applyDailyAlarm()) );
    connect( qApp, SIGNAL(clockChanged(bool)), this, SLOT(changeClock(bool)) );

    connect( alarmEnabled, SIGNAL(toggled(bool)), this, SLOT(setDailyEnabled(bool)) );
    connect( changeAlarmDaysButton, SIGNAL(clicked()), this, SLOT(changeAlarmDays()) );

    QSettings cConfig("Trolltech","Clock"); // No tr
    cConfig.beginGroup( "Daily Alarm" );

    QStringList exclDays = cConfig.value( "ExcludeDays").toStringList();
    for (int i=Qt::Monday; i<=Qt::Sunday; i++) {
        daysSettings.insert(i, !exclDays.contains(QString::number(i)));
    }
    resetAlarmDaysText();

    initEnabled = cConfig.value("Enabled", false).toBool();
    alarmEnabled->setChecked( initEnabled );
    int m = cConfig.value( "Minute", 0 ).toInt();
    int h = cConfig.value( "Hour", 7 ).toInt();

    if (ampm)
        alarmTimeEdit->setDisplayFormat("h:mm ap");
    else
        alarmTimeEdit->setDisplayFormat("hh:mm");
    alarmTimeEdit->setTime( QTime( h, m ) );

    connect( alarmTimeEdit, SIGNAL(editingFinished()), this, SLOT(applyDailyAlarm())) ;

    alarmDaysEdit->installEventFilter(this);

    init = true;
    QtopiaApplication::instance()->registerRunningTask(QLatin1String("waitForTimer"), this);
}

Alarm::~Alarm()
{
}

void Alarm::changeClock( bool a )
{
    //change display format (whether or not we want am/pm)
    if ( ampm != a ) {
        ampm = a;
        if (ampm)
            alarmTimeEdit->setDisplayFormat("h:mm ap");
        else
            alarmTimeEdit->setDisplayFormat("hh:mm");
    }
}

void Alarm::triggerAlarm(const QDateTime &when, int type)
{
    QTime theTime( when.time() );
    if ( type == magic_daily ) {
        QString ts = QTimeString::localHM(theTime);
        QString msg = ts + "\n" + tr( "(Daily Alarm)" );
        setRingPriority(true);
        Qtopia::soundAlarm();
        alarmCount = 0;
        alarmt->start( 2000 );
        if ( !alarmDlg ) {
            alarmDlg = new QDialog( this );
            alarmDlg->setWindowTitle( tr("Clock") );
            QVBoxLayout *vb = new QVBoxLayout(alarmDlg);
            vb->setMargin(6);
            vb->addStretch(1);
            QLabel *l = new QLabel( alarmDlg );
            QIcon icon(":icon/alarmbell");
            QPixmap pm = icon.pixmap(icon.actualSize(QSize(100,100)));
            l->setPixmap(pm);
            l->setAlignment( Qt::AlignCenter );
            vb->addWidget(l);
            alarmDlgLabel = new QLabel( msg, alarmDlg );
            alarmDlgLabel->setAlignment( Qt::AlignCenter );
            vb->addWidget(alarmDlgLabel);
            vb->addStretch(1);
        } else {
            alarmDlgLabel->setText(msg);
        }
        // Set for tomorrow, so user wakes up every day, even if they
        // don't confirm the dialog.
        applyDailyAlarm();
        if ( !alarmDlg->isVisible() ) {
            QtopiaApplication::execDialog(alarmDlg);
            alarmt->stop();
            setRingPriority(false);
        }
    }
}

void Alarm::setDailyEnabled(bool enableDaily)
{
    alarmEnabled->setChecked( enableDaily );
    applyDailyAlarm();
}

void Alarm::alarmTimeout()
{
    if ( alarmCount < 20 ) {
        Qtopia::soundAlarm();
        alarmCount++;
    } else {
        setRingPriority(false);
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

    while ( when < now || daysSettings[dow] == false ) {
        when = when.addDays( 1 );
        dow = when.date().dayOfWeek();
        if ( ++count > 7 )
            return QDateTime();
    }

    return when;
}

void Alarm::applyDailyAlarm()
{
    if ( !init )
        return;

    int minute = alarmTimeEdit->time().minute();
    int hour = alarmTimeEdit->time().hour();

    QSettings config("Trolltech","Clock");
    config.beginGroup( "Daily Alarm" );
    config.setValue( "Hour", hour );
    config.setValue( "Minute", minute );

    bool enableDaily = alarmEnabled->isChecked();
    config.setValue( "Enabled", enableDaily );

    QStringList exclDays;
    for (int i=1; i<=7; i++) {
        if ( !daysSettings.value(i, false) ) {
            exclDays << QString::number( i );
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
    if ( alarmEnabled->isChecked() && exclDays.size() < 7 ) {
        QDateTime when = nextAlarm( hour, minute );
        Qtopia::addAlarm(when, "QPE/Application/clock",
                            "alarm(QDateTime,int)", magic_daily);
    }
}

bool Alarm::eventFilter(QObject *o, QEvent *e)
{
    if (o == alarmDaysEdit && alarmEnabled->isChecked()) {
        if (e->type() == QEvent::MouseButtonRelease) {
            changeAlarmDays();
            return true;
        } else if (e->type() == QEvent::Resize) {
            resetAlarmDaysText();
        }
    }
    return false;
}

QString Alarm::getAlarmDaysText() const
{
    int day;

    QList<int> alarmDays;
    for (day=Qt::Monday; day<=Qt::Sunday; day++) {
        if (daysSettings[day])
            alarmDays << day;
    }

    int alarmDaysCount = alarmDays.size();
    if (alarmDaysCount == 7) {
        return tr("Every day");

    } else if (alarmDaysCount == 5 &&
                alarmDays[0] == Qt::Monday &&
                alarmDays.last() == Qt::Friday) {

        return tr("Weekdays");

    } else if (alarmDaysCount == 2 &&
                alarmDays[0] == Qt::Saturday &&
                alarmDays[1] == Qt::Sunday) {

        return tr("Weekends");

    } else {
        if (alarmDaysCount == 0)
            return QLatin1String("");

        QStringList dayStrings;
        if (alarmDaysCount == 1) {
            return QTimeString::nameOfWeekDay(alarmDays[0],
                        QTimeString::Long);
        } else {
            for (day=0; day<alarmDaysCount; day++) {
                dayStrings << QTimeString::nameOfWeekDay(alarmDays[day],
                        QTimeString::Medium);
            }
            // move Sunday to front if necessary
            if (!weekStartsMonday && alarmDays.last() == Qt::Sunday)
                dayStrings.insert(0, dayStrings.takeLast());
            return dayStrings.join(", ");
        }
    }
}

void Alarm::resetAlarmDaysText()
{
    QFontMetrics fm(alarmDaysEdit->font());
    alarmDaysEdit->setText(fm.elidedText(getAlarmDaysText(), Qt::ElideRight, alarmDaysEdit->width()));
    alarmDaysEdit->home(false);
}

void Alarm::changeAlarmDays()
{
    int day;
    QDialog dlg;
    QVBoxLayout layout;

    QHash<int, QCheckBox *> checkboxes;
    QCheckBox *c;
    for (day=Qt::Monday; day<=Qt::Sunday; day++) {
        c = new QCheckBox(QTimeString::nameOfWeekDay(day, QTimeString::Long));
        c->setChecked(daysSettings[day]);
        checkboxes.insert(day, c);

        if (day == Qt::Sunday && !weekStartsMonday)
            layout.insertWidget(0, c);
        else
            layout.addWidget(c);
    }

    layout.setSpacing(9);
    layout.setMargin(6);
    dlg.setLayout(&layout);
    dlg.setWindowTitle(tr("Set alarm days"));

    if (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted) {
        bool foundChecked = false;
        for (day=Qt::Monday; day<=Qt::Sunday; day++) {
            if (checkboxes[day]->isChecked()) {
                foundChecked = true;
                break;
            }
        }

        // don't change previous alarm daysSettings if nothing has been checked
        // and the alarm is going to be disabled
        if (foundChecked) {
            for (day=Qt::Monday; day<=Qt::Sunday; day++)
                daysSettings[day] = checkboxes[day]->isChecked();
        }

        // if no daysSettings have been set, disable the alarm
        setDailyEnabled(foundChecked);
        resetAlarmDaysText();
    }
}


#include "alarm.moc"
