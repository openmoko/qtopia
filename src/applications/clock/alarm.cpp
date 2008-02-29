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

#include "alarm.h"

#include <qtopia/qpeapplication.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/config.h>
#include <qtopia/timestring.h>
#include <qtopia/alarmserver.h>
#include <qtopia/sound.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>
#ifdef QTOPIA_PHONE
# include <qtopia/contextbar.h>
# include <qtopia/contextmenu.h>
#endif

#include <qaccel.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qlineedit.h>
#include <qtimer.h>

static const int magic_daily = 2292922;
static const int magic_countdown = 2292923;

#if 0
static void toggleScreenSaver( bool on )
{
    QPEApplication::setTempScreenSaverMode(on ? QPEApplication::Enable : QPEApplication::DisableSuspend);  
}
#endif

class MySpinBox : public QSpinBox
{
public:
    QLineEdit *lineEdit() const {
	return editor();
    }
};

Alarm::Alarm( QWidget * parent, const char *, WFlags f )
    : AlarmBase( parent, "clock", f ), swatch_splitms(99), init(FALSE) // No tr
{
    alarmDlg = 0;
    alarmDlgLabel = 0;
    dayBtn = new QToolButton * [7];

    Config config( "qpe" );
    config.setGroup("Time");
    ampm = config.readBoolEntry( "AMPM" );
    onMonday = config.readBoolEntry( "MONDAY" );

    applyAlarmTimer = new QTimer( this );
    connect( applyAlarmTimer, SIGNAL(timeout()),
	this, SLOT(applyDailyAlarm()) );

    alarmt = new QTimer( this );
    connect( alarmt, SIGNAL(timeout()), SLOT(alarmTimeout()) );

    connect( qApp, SIGNAL(timeChanged()), SLOT(applyDailyAlarm()) );
    connect( qApp, SIGNAL(clockChanged(bool)), this, SLOT(changeClock(bool)) );

    cdGroup->hide(); // XXX implement countdown timer.

    connect( dailyHour, SIGNAL(valueChanged(int)), this, SLOT(scheduleApplyDailyAlarm()) );
    connect( dailyMinute, SIGNAL(valueChanged(int)), this, SLOT(setDailyMinute(int)) );
    connect( dailyAmPm, SIGNAL(activated(int)), this, SLOT(setDailyAmPm(int)) );
    connect( dailyEnabled, SIGNAL(toggled(bool)), this, SLOT(enableDaily(bool)) );

    Config cConfig( "Clock" ); // No tr
    cConfig.setGroup( "Daily Alarm" );

    TimeString::Length len = TimeString::Short;
    int approxSize = QFontMetrics(QFont()).width(" Wed ") * 7;
    if ( QApplication::desktop()->width() > approxSize )
	len = TimeString::Medium;

    int i;
    QHBoxLayout *hb = new QHBoxLayout( daysFrame );
    for ( i = 0; i < 7; i++ ) {
	dayBtn[i] = new QToolButton( daysFrame );
	hb->addWidget( dayBtn[i] );
	dayBtn[i]->setToggleButton( TRUE );
	dayBtn[i]->setOn( TRUE );
	dayBtn[i]->setFocusPolicy( StrongFocus );
	connect( dayBtn[i], SIGNAL(toggled(bool)), this, SLOT(scheduleApplyDailyAlarm()) );
    }

    for ( i = 0; i < 7; i++ )
	dayBtn[dayBtnIdx(i+1)]->setText( TimeString::localDayOfWeek(i + 1, len) );

    QStringList exclDays = cConfig.readListEntry( "ExcludeDays", ',' );
    QStringList::Iterator it;
    for ( it = exclDays.begin(); it != exclDays.end(); ++it ) {
	int d = (*it).toInt();
	if ( d >= 1 && d <= 7 )
	    dayBtn[dayBtnIdx(d)]->setOn( FALSE );
    }

    initEnabled = cConfig.readBoolEntry("Enabled", FALSE);
    dailyEnabled->setChecked( initEnabled );
    int m = cConfig.readNumEntry( "Minute", 0 );
    dailyMinute->setValue( m );
//    dailyMinute->setPrefix( m <= 9 ? "0" : "" );
    int h = cConfig.readNumEntry( "Hour", 7 );
    if ( ampm ) {
	if (h > 12) {
	    h -= 12;
	    dailyAmPm->setCurrentItem( 1 );
	}
	if (h == 0) h = 12;
	dailyHour->setMinValue( 1 );
	dailyHour->setMaxValue( 12 );
    } else {
	dailyAmPm->hide();
    }
    dailyHour->setValue( h );

    connect( ((MySpinBox*)dailyHour)->lineEdit(), SIGNAL(textChanged(const QString&)),
	    this, SLOT(dailyEdited()) );
    connect( ((MySpinBox*)dailyMinute)->lineEdit(), SIGNAL(textChanged(const QString&)),
	    this, SLOT(dailyEdited()) );

    init = TRUE;
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
	    if (dailyAmPm->currentItem() == 1 )
		hour += 12;
	    dailyHour->setMinValue( 0 );
	    dailyHour->setMaxValue( 23 );
	    dailyAmPm->hide();
	} else {
	    if (hour > 12) {
		hour -= 12;
		dailyAmPm->setCurrentItem( 1 );
	    }
	    if (hour == 0) hour = 12;
	    dailyHour->setMinValue( 1 );
	    dailyHour->setMaxValue( 12 );
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

void Alarm::setDailyMinute( int m )
{
    dailyMinute->setPrefix( m <= 9 ? "0" : "" );
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
	    bool pm = FALSE;
	    int h = theTime.hour();
	    if (h > 12) {
		h -= 12;
		pm = TRUE;
	    }
	    if (h == 0) h = 12;
	    ts.sprintf( "%02d:%02d %s", h, theTime.minute(), pm?"PM":"AM" );
	} else {
	    ts.sprintf( "%02d:%02d", theTime.hour(), theTime.minute() );
	}
	msg += ts;
	Sound::soundAlarm();
	alarmCount = 0;
	alarmt->start( 5000 );
	if ( !alarmDlg ) {
	    alarmDlg = new QDialog( this->isVisible() ? this : 0, 0, TRUE,
		    WStyle_Customize | WStyle_StaysOnTop |
		    WStyle_NormalBorder | WStyle_Title );

	    alarmDlg->setCaption( tr("Clock") );
	    QVBoxLayout *vb = new QVBoxLayout( alarmDlg, 6 );
	    QHBoxLayout *hb = new QHBoxLayout( vb );
	    QLabel *l = new QLabel( alarmDlg );
	    QPixmap pm(Resource::loadPixmap("alarmbell"));
	    l->setPixmap(pm);
	    l->setFixedWidth(pm.width());
	    hb->addWidget(l);
	    alarmDlgLabel = new QLabel( msg, alarmDlg );
	    alarmDlgLabel->setAlignment( AlignCenter );
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
	    QPEApplication::execDialog(alarmDlg);
	    alarmt->stop();
	}
    } else if ( type == magic_countdown ) {
	// countdown
	Sound::soundAlarm();
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
	Sound::soundAlarm();
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
    while ( when < now || !dayBtn[dayBtnIdx(dow)]->isOn() ) {
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
    applyAlarmTimer->start( 5000, TRUE );
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
	if (dailyAmPm->currentItem() == 1 )
	    hour += 12;
    }

    Config config( "Clock" );
    config.setGroup( "Daily Alarm" );
    config.writeEntry( "Hour", hour );
    config.writeEntry( "Minute", minute );

    bool enableDaily = dailyEnabled->isChecked();
    config.writeEntry( "Enabled", enableDaily );

    QString exclDays;
    int exclCount = 0;
    for ( int i = 1; i <= 7; i++ ) {
	if ( !dayBtn[dayBtnIdx(i)]->isOn() ) {
	    if ( !exclDays.isEmpty() )
		exclDays += ",";
	    exclDays += QString::number( i );
	    exclCount++;
	}
    }
    config.writeEntry( "ExcludeDays", exclDays );
    config.write();

    if (enableDaily != initEnabled) {
	QCopEnvelope e("QPE/System", "dailyAlarmEnabled(int)");
	e << enableDaily;
	initEnabled = enableDaily;
    }

    AlarmServer::deleteAlarm(QDateTime(), "QPE/Application/clock",
	    "alarm(QDateTime,int)", magic_daily);
    if ( dailyEnabled->isChecked() && exclCount < 7 ) {
	QDateTime when = nextAlarm( hour, minute );
	AlarmServer::addAlarm(when, "QPE/Application/clock",
			    "alarm(QDateTime,int)", magic_daily);
    }
}

bool Alarm::validDaysSelected(void) const
{
    for ( int i = 1; i <= 7; i++ ) {
	if ( dayBtn[dayBtnIdx(i)]->isOn() ) {
	    return TRUE;
	}
    }
    return FALSE;
}

bool Alarm::spinBoxValid( QSpinBox *sb )
{
    bool valid = TRUE;
    QString tv = sb->text();
    for ( uint i = 0; i < tv.length(); i++ ) {
	if ( !tv[0].isDigit() )
	    valid = FALSE;
    }
    bool ok = FALSE;
    int v = tv.toInt( &ok );
    if ( !ok )
	valid = FALSE;
    if ( v < sb->minValue() || v > sb->maxValue() )
	valid = FALSE;

    return valid;
}

#ifdef QTOPIA_PHONE

void Alarm::keyPressEvent(QKeyEvent *ke)
{
    int i;
    switch(ke->key()) {
	case Key_Left:
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
	case Key_Right:
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
