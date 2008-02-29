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

#include "repeatentry.h"

#include <qtopia/datepicker.h>
#include <qtopia/calendar.h>

#include <qtopia/qpeapplication.h>

#include <qtabbar.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qhbuttongroup.h>
#include <qvbuttongroup.h>
#include <qtoolbutton.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qwidgetstack.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qsimplerichtext.h>
#include <qpainter.h>
#include <qfocusdata.h>

#ifdef QTOPIA_PHONE
static const TimeString::Length TIMESTRING_LENGTH = TimeString::Medium;
#else
static const TimeString::Length TIMESTRING_LENGTH = TimeString::Long;
#endif

int currentID( QButtonGroup *group )
{
    for (int i = 0; i < group->count(); i++) {
	if (group->find(i)->isOn())
	    return i;
    }
    return -1;
}

class RichLabel : public QLabel
{
public:
    RichLabel(QWidget *p) : QLabel(p) { }
protected:
    void drawContents(QPainter *p)
    {
	QSimpleRichText rt(text(), p->font());
	rt.setWidth(width());
#if (QT_VERSION-0 >= 0x030000)
	rt.draw(p, pos().x(),pos().y(), geometry(), colorGroup());
#else
	rt.draw(p, 0,0, QRegion(geometry()), palette());
#endif
    }
};

static void nextButton( QWidget **btns, int size, bool prev = FALSE )
{
    int move = 1;
    if ( prev )
	move = size - 1;

    for ( int i = 0; i < size; i++ ) {
	if ( btns[i]->hasFocus() ) {
	    int index = i;
	    do {
		index = (index + move) % size;
	    } while ( !btns[index]->isEnabled() );
	    btns[index]->setFocus();
	    break;
	}
    }
}

// ====================================================================

class WeekGroup : public QButtonGroup
{
    Q_OBJECT

public:
    WeekGroup(int dayOfWeek, bool startOnMonday, QWidget *parent = 0);

    ~WeekGroup() {}

    void toggleDay(int, bool);

public slots:
    void setStartOnMonday(bool b);

signals:
    void dayToggled(int, bool);

protected:
    void keyPressEvent( QKeyEvent *e );

private slots:
    void setDay(int);

private:
    QToolButton *dayBtn[7];
};

// ====================================================================

WeekGroup::WeekGroup(int dayOfWeek, bool startOnMonday, QWidget *parent)
    : QButtonGroup(tr("Repeat On"), parent)
{
    setColumnLayout(0, Qt::Vertical);
    layout()->setSpacing(0);
    layout()->setMargin(0);
    setExclusive(FALSE);
    layout()->setSpacing(0);

    QHBoxLayout *fe = new QHBoxLayout(layout());
    fe->setAlignment( Qt::AlignTop );
    fe->setSpacing(0);
    fe->setMargin(6);

    TimeString::Length len = TimeString::Short;
    int approxSize = QFontMetrics(QFont()).width(" Wed ") * 7;
    if ( QApplication::desktop()->width() > approxSize )
	len = TimeString::Medium;

    int j = 0;
    if ( !startOnMonday )
	j = 6;
    for (int i = 0; i <= 6; i++) {
	dayBtn[i] = new QToolButton(this);
	dayBtn[i]->setToggleButton(TRUE);
	if ( j == (dayOfWeek - 1) ) {
	    dayBtn[i]->setEnabled(FALSE);
	    //b1->setOn(TRUE);
	}
	dayBtn[i]->setText(TimeString::localDayOfWeek(j + 1, len));
	insert(dayBtn[i], j + 1);
	fe->addWidget(dayBtn[i]);

	dayBtn[i]->setFocusPolicy(QWidget::StrongFocus);
	if ( ++j == 7 )
	    j = 0;
    }

    connect(this, SIGNAL(clicked(int)), this, SLOT(setDay(int)));
}

void WeekGroup::setStartOnMonday(bool /* b */)
{
    /*
    if (onMonday == b)
	return;
    onMonday = b;
    if (onMonday) {
	first->hide();
	last->show();
    } else {
	last->hide();
	first->show();
    }
    */
}

void WeekGroup::toggleDay(int i, bool b)
{
    /*
    if (i < 1 || i > 6) {
	// toggle sundays;
	first->setOn(b);
	last->setOn(b);
    } else {
    */
	if (find(i))
	    ((QToolButton *)find(i))->setOn(b);
    //}
}

// internal
void WeekGroup::setDay(int i)
{
    QToolButton *bt = (QToolButton *)find(i);
    if (bt) {
	bool on = bt->isOn();
	/*
	if (i < 1 || i > 6) {
	    first->setOn(on);
	    last->setOn(on);
	    emit dayToggled(7, on);
	} else {
	*/
	    emit dayToggled(i, on);
	//}
    }
}

void WeekGroup::keyPressEvent( QKeyEvent *e )
{
    //qDebug( "key %d (up %d, down %d)", e->key(), Key_Up, Key_Down );
#ifdef QTOPIA_PHONE
    switch ( e->key() ) {
	case Key_Left:
	    nextButton( (QWidget**)dayBtn, 7, TRUE );
	    break;
	case Key_Right:
	    nextButton( (QWidget**)dayBtn, 7 );
	    break;
	default:
	    QButtonGroup::keyPressEvent( e );
	    break;
    }
#else
    QButtonGroup::keyPressEvent( e );
#endif
}

// ====================================================================

RepeatEntry::RepeatEntry( bool startOnMonday, const PimEvent &rp,
	QWidget *parent, const char *name, bool modal, WFlags fl)
    : QDialog(parent, "event-repeat", modal, fl), mEvent(rp)
{
    Q_UNUSED( name );
#ifdef QTOPIA_PHONE
    setCaption(tr("Repeat"));
#else
    setCaption(tr("Repeating Event"));
#endif
    // RepeatType
    QVBoxLayout *layout = new QVBoxLayout(this);
#ifdef QTOPIA_PHONE
    layout->setMargin(0);
    layout->setSpacing(0);
#else
    layout->setMargin(6);
    layout->setSpacing(6);
#endif

    QHBoxLayout *buttonLayout = new QHBoxLayout(layout);
    buttonLayout->setSpacing(0);
    typeSelector = new QButtonGroup(this);
    typeSelector->hide();
    typeSelector->setExclusive(TRUE);
    typeBtn[0] = new QPushButton(tr("Day","Day, not date"), this);
    typeBtn[0]->setToggleButton(TRUE);
    typeBtn[0]->setFocusPolicy(StrongFocus);
    typeBtn[0]->setAutoDefault(FALSE);
    buttonLayout->addWidget(typeBtn[0]);
    typeSelector->insert(typeBtn[0], 0);
    typeBtn[1] = new QPushButton(tr("Week"), this);
    typeBtn[1]->setToggleButton(TRUE);
    typeBtn[1]->setFocusPolicy(StrongFocus);
    typeBtn[1]->setAutoDefault(FALSE);
    buttonLayout->addWidget(typeBtn[1]);
    typeSelector->insert(typeBtn[1], 1);
    typeBtn[2] = new QPushButton(tr("Month"), this);
    typeBtn[2]->setToggleButton(TRUE);
    typeBtn[2]->setFocusPolicy(StrongFocus);
    typeBtn[2]->setAutoDefault(FALSE);
    buttonLayout->addWidget(typeBtn[2]);
    typeSelector->insert(typeBtn[2], 2);
    typeBtn[3] = new QPushButton(tr("Year"), this);
    typeBtn[3]->setToggleButton(TRUE);
    typeBtn[3]->setFocusPolicy(StrongFocus);
    typeBtn[3]->setAutoDefault(FALSE);
    buttonLayout->addWidget(typeBtn[3]);
    typeSelector->insert(typeBtn[3], 3);

    switch(mEvent.repeatType()) {
	case PimEvent::NoRepeat:
	case PimEvent::Daily:
	    typeSelector->find(0)->toggle();
	    break;
	case PimEvent::Weekly:
	    typeSelector->find(1)->toggle();
	    break;
	case PimEvent::MonthlyDate:
	case PimEvent::MonthlyDay:
	case PimEvent::MonthlyEndDay:
	    typeSelector->find(2)->toggle();
	    break;
	case PimEvent::Yearly:
	    typeSelector->find(3)->toggle();
	    break;
    }

    layout->addWidget(typeSelector);

    // Frequency and end date
    fStack = new QWidgetStack(this);
    fStack->addWidget(new QWidget(fStack), 0);
#ifdef QTOPIA_PHONE
    fStack->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
#endif

    QWidget *freqBox = new QWidget(fStack);
    QVBoxLayout *subLayout = new QVBoxLayout(freqBox);
    QHBoxLayout *subLayout2 = new QHBoxLayout(subLayout);
#ifndef QTOPIA_PHONE
    subLayout2->setSpacing(6);
#endif

    everyLabel = new QLabel("", freqBox);
    subLayout2->addWidget(everyLabel);
    everyLabel->setAlignment(AlignRight|AlignVCenter);
    freqSelector = new QSpinBox(freqBox);
    freqSelector->setMinValue(1);
    subLayout2->addWidget(freqSelector);
    typeLabel = new QLabel(freqBox);
    subLayout2->addWidget(typeLabel);

    freqSelector->setValue(mEvent.frequency());

    freqSelector->setValue(mEvent.frequency());

    /*
    endSelect->setDate(mEvent.repeatTill());
    if (mEvent.hasRepeat()) {
	endSelect->setEnabled(!mEvent.repeatForever());
	hasEndCheck->setChecked(mEvent.repeatForever());
    } else {
	endSelect->setEnabled(FALSE);
	hasEndCheck->setChecked(TRUE);
    }
    */

    fStack->addWidget(freqBox, 1);

    layout->addWidget(fStack);

    // Now for the day select/ month type select part
    subStack = new QWidgetStack(this);
    subStack->addWidget(new QWidget(subStack), 0);

    weekGroup = new WeekGroup(mEvent.start().date().dayOfWeek(), startOnMonday, subStack);
    subStack->addWidget(weekGroup, 1);

    monthGroup = new QVButtonGroup(tr("Repeat On"), subStack);
    monthGroup->setExclusive(TRUE);
#ifdef QTOPIA_PHONE
    monthGroup->layout()->setSpacing(0);
    monthGroup->layout()->setMargin(6);
#endif

    QString strEndWeekDay1 = tr("the last %1.","eg. %1 = Friday");
    strEndWeekDay = tr("the %1 last %2.","eg. %1 last %2 = 2nd last Friday");

    strDate = tr("day %1 of the month.","eg. %1 = 3")
	.arg(mEvent.start().date().day());

    strWeekDay = tr("the %1 %2.","eg. %1 %2 = 2nd Friday")
	.arg(trSmallOrdinal(
		    Calendar::weekInMonth(mEvent.start().date())))
	.arg(TimeString::localDayOfWeek(mEvent.start().date(), TIMESTRING_LENGTH));

    int fromEndOfWeek = mEvent.start().date().daysInMonth()
	- mEvent.start().date().day();
    fromEndOfWeek = fromEndOfWeek > 0 ? fromEndOfWeek / 7 + 1 : 1;

    strEndWeekDay =
	((fromEndOfWeek > 1)
	    ? strEndWeekDay.arg( trSmallOrdinal(fromEndOfWeek) )
	    : strEndWeekDay1)
	.arg( TimeString::localDayOfWeek( mEvent.start().date(), TIMESTRING_LENGTH ) );

    QRadioButton *r1 = new QRadioButton(strDate, monthGroup);
    r1 = new QRadioButton(strWeekDay, monthGroup);
    r1 = new QRadioButton(strEndWeekDay, monthGroup);



    subStack->addWidget(monthGroup, 2);

    layout->addWidget(subStack);

    //QSpacerItem *si = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    //layout->addItem(si);
    // and last but not least, the label.. later.
    //QFrame
#ifndef QTOPIA_PHONE
    QFrame *fr = new QFrame(this);
    fr->setMinimumSize(QSize(40, 50));
    fr->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
    fr->setFrameStyle(QFrame::Box | QFrame::Sunken);
    subLayout = new QVBoxLayout(fr);
    subLayout->setSpacing(1);
    subLayout->setMargin(5);
    descLabel = new RichLabel(fr);
    descLabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
    subLayout->addWidget(descLabel);
    layout->addWidget(fr);
#endif

    // pretend we just changed repeat types.
    setRepeatType(currentID(typeSelector));

#ifdef QTOPIA_DESKTOP
    QHBoxLayout *hb = new QHBoxLayout(layout);
    QSpacerItem *s = new QSpacerItem(10,1);
    QPushButton *okBtn = new QPushButton(tr("OK"), this);
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"), this);
    hb->addItem(s);
    hb->addWidget(okBtn);
    hb->addWidget(cancelBtn);

    connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
#endif

    // Connection time.
    connect(typeSelector, SIGNAL(clicked(int)), this, SLOT(setRepeatType(int)));
    connect(freqSelector, SIGNAL(valueChanged(int)),
	    this, SLOT(setFrequency(int)));
    connect(weekGroup, SIGNAL(dayToggled(int,bool)),
	    this, SLOT(setRepeatOnWeekDay(int,bool)));
    connect(monthGroup, SIGNAL(clicked(int)),
	    this, SLOT(setSubRepeatType(int)));

#ifndef QTOPIA_PHONE
    resize(100,100);
#endif
}

RepeatEntry::~RepeatEntry()
{
}

QString RepeatEntry::trSmallOrdinal(int n) const
{
    if ( n == 1 ) return tr("first","eg. first Friday of month");
    else if ( n == 2 ) return tr("second");
    else if ( n == 3 ) return tr("third");
    else if ( n == 4 ) return tr("fourth");
    else if ( n == 5 ) return tr("fifth");
    else return QString::number(n);
}

void RepeatEntry::refreshLabels()
{
    QString type;
    switch (mEvent.repeatType())
    {
	case PimEvent::NoRepeat:
#ifndef QTOPIA_PHONE
	    descLabel->setText(tr("Don't repeat."));
#endif
	    break;
	case PimEvent::Daily:
#ifndef QTOPIA_PHONE
	    if (mEvent.frequency() == 1)
		descLabel->setText(tr("Repeat every day."));
	    else
		descLabel->setText(tr("Repeat every %1 days.").arg(mEvent.frequency()));
#endif
	    type = tr("Every %1 day(s)");
	    break;
	case PimEvent::Weekly:
	    {
		type = tr("Every %1 week(s)");
#ifndef QTOPIA_PHONE
		// first work out how many items in the list of weeks
		int i;
		int count = 0;
		int dow = mEvent.start().date().dayOfWeek();
		int firstday = dow;
		int lastday = dow;
		for (i = 0; i < 7; i++) {
		    if (mEvent.repeatOnWeekDay(dow)) {
			count++;
			lastday = dow;
		    }
		    dow++;
		    if (dow > 7) dow = 1;
		}
		// pick a string
		QString buf;
		if (mEvent.frequency() == 1) {
		    switch(count) {
			default:
			case 1:
			    buf = tr("Repeat every week on %1.", "e.g. %1 = Monday" );
			    break;
			case 2:
			    buf = tr("Repeat every week on %1 and %2.", "e.g. %1 = Monday, %2 = Wednesday..." );
			    break;
			case 3:
			    buf = tr("Repeat every week on %1, %2 and %3.", "e.g. %1 = Monday, %2 = Wednesday..." );
			    break;
			case 4:
			    buf = tr("Repeat every week on %1, %2, %3 and %4.", "e.g. %1 = Monday, %2 = Wednesday..." );
			    break;
			case 5:
			    buf = tr("Repeat every week on %1, %2, %3, %4 and %5.", "e.g. %1 = Monday, %2 = Wednesday..." );
			    break;
			case 6:
			    buf = tr("Repeat every week on %1, %2, %3, %4, %5 and %6.", "e.g. %1 = Monday, %2 = Wednesday..." );
			    break;
			case 7:
			    buf = tr("Repeat every day.");
			    break;
		    }
		} else {
		    switch(count) {
			default:
			case 1:
			    buf = tr("Repeat every %1 weeks on %2.", "e.g. %1 = 3, %2 = Monday" );
			    break;
			case 2:
			    buf = tr("Repeat every %1 weeks on %2 and %3.", "e.g. %1 = 3, %2 = Monday, %3 = Wednesday..." );
			    break;
			case 3:
			    buf = tr("Repeat every %1 weeks on %2, %3 and %4.", "e.g. %1 = 3, %2 = Monday, %3 = Wednesday..." );
			    break;
			case 4:
			    buf = tr("Repeat every %1 weeks on %2, %3, %4 and %5.", "e.g. %1 = 3, %2 = Monday, %3 = Wednesday..." );
			    break;
			case 5:
			    buf = tr("Repeat every %1 weeks on %2, %3, %4, %5 and %6.", "e.g. %1 = 3, %2 = Monday, %3 = Wednesday..." );
			    break;
			case 6:
			    buf = tr("Repeat every %1 weeks on %2, %3, %4, %5, %6 and %7.", "e.g. %1 = 3, %2 = Monday, %3 = Wednesday..." );
			    break;
			case 7:
			    // more complex.... 
			    buf = tr("Repeat every %1 weeks from %2 to %3.", 
				    "e.g. %1 = 3, %2 = Monday, %3 = Sunday.  "
				    "Describes an event that occurs each day of the "
				    "week for a full week, starting on %2 and ending on %3, every %1 weeks." );
			    break;
		    }
		    buf = buf.arg(mEvent.frequency());
		    if (count == 7) {
			// fill in aditional days now.
			buf = buf.arg( TimeString::localDayOfWeek(firstday, TimeString::Long ) );
			buf = buf.arg( TimeString::localDayOfWeek(lastday, TimeString::Long ) );
		    }
		}

		if (count != 7) {
		    dow = mEvent.start().date().dayOfWeek();
		    for (i = 0; i < 7; i++) {
			if (mEvent.repeatOnWeekDay(dow))
			    buf = buf.arg( TimeString::localDayOfWeek(dow, TimeString::Long ) );
			dow++;
			if (dow > 7) dow = 1;
		    }
		}
		descLabel->setText(buf);
#endif
	    }
	    break;
	case PimEvent::MonthlyDate:
#ifndef QTOPIA_PHONE
	    if (mEvent.frequency() == 1)
		descLabel->setText(
			tr("Repeat every month on day %1 of the month.","eg. %1 = 3")
			.arg(mEvent.start().date().day())
			);
	    else
		descLabel->setText(
			tr("Repeat every %1 months on day %2 of the month.", "eg. %1 = 4, %2 = 3rd")
			.arg(mEvent.frequency())
			.arg(mEvent.start().date().day())
			);
#endif
	    type = tr("Every %1 month(s)");
	    break;
	case PimEvent::MonthlyDay:
#ifndef QTOPIA_PHONE
	    if (mEvent.frequency() == 1)
		descLabel->setText(tr("Repeat every month on the %1 %2 of the month.", "eg. %1 %2 = 2nd Friday")
			.arg(trSmallOrdinal(
				Calendar::weekInMonth(mEvent.start().date())))
			.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
			);
	    else
		descLabel->setText(tr("Repeat every %1 months on the %2 %3 of the month.", "eg. %1 = 4, %2 %3 = 2nd Friday")
			.arg(mEvent.frequency()) 
			.arg(trSmallOrdinal(
				Calendar::weekInMonth(mEvent.start().date())))
			.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
			);
#endif
	    type = tr("Every %1 month(s)");
	    break;
	case PimEvent::MonthlyEndDay:
	    {
#ifndef QTOPIA_PHONE
		int fromEndOfWeek = mEvent.start().date().daysInMonth()
		    - mEvent.start().date().day();
		fromEndOfWeek = fromEndOfWeek > 0 ? fromEndOfWeek / 7 + 1 : 1;

		if (mEvent.frequency() == 1) {
		    if (fromEndOfWeek == 1) {
			descLabel->setText(
				tr("Repeat every month on the last %1 of the month.","eg. %1 = Friday")
				.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
				);
		    } else {
			descLabel->setText(
				tr("Repeat every month on %1 last %2 of the month.", "eg. %1 = 2nd, %2 = Friday")
				.arg( trSmallOrdinal(fromEndOfWeek) )
				.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
				);
		    }
		} else {
		    if (fromEndOfWeek == 1) {
			descLabel->setText(
				tr("Repeat every %2 months on the last %1 of the month.","eg. %1 = Friday, %2 = 4")
				.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
				.arg(mEvent.frequency()) 
				);
		    } else {
			descLabel->setText(
				tr("Repeat every %3 months on %1 last %2 of the month.", "eg. %1 = 2nd, %2 = Friday, %3 = 4")
				.arg( trSmallOrdinal(fromEndOfWeek) )
				.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
				.arg(mEvent.frequency()) 
				);
		    }
		}
#endif
		type = tr("Every %1 month(s)");
	    }
	    break;
	case PimEvent::Yearly:
#ifndef QTOPIA_PHONE
	    if (mEvent.frequency() == 1)
		descLabel->setText(tr("Repeat every year."));
	    else
		descLabel->setText(tr("Repeat every %1 years.").arg(mEvent.frequency()));
#endif
	    type = tr("Every %1 year(s)");
	    break;
	default:
#ifndef QTOPIA_PHONE
	    descLabel->setText("Bug <2316>"); // No tr
#endif
	    break;
    }
    int spinbox = type.find("%1");
    everyLabel->setText(type.left(spinbox));
    typeLabel->setText(type.mid(spinbox+2));
}

PimEvent RepeatEntry::event() const
{
    return mEvent;
}

void RepeatEntry::setRepeatType(int t)
{
    int i;
    switch (t) {
	case 0:
	    mEvent.setRepeatType(PimEvent::Daily);
	    fStack->raiseWidget(1);
	    subStack->raiseWidget(0);
	    break;
	case 1:
	    mEvent.setRepeatType(PimEvent::Weekly);
	    fStack->raiseWidget(1);
	    subStack->raiseWidget(1);
	    for(i = 1; i <= 7; i++)
		weekGroup->toggleDay(i, mEvent.repeatOnWeekDay(i));
	    break;
	case 2:
	    // because has sub type, check if we are initializing by
	    // if we are already in this tab.  if not, then go the default.
	    switch (mEvent.repeatType()) {
		case PimEvent::MonthlyDate:
		    ((QRadioButton *)monthGroup->find(0))->setChecked(TRUE);
		    break;
		case PimEvent::MonthlyDay:
		    ((QRadioButton *)monthGroup->find(1))->setChecked(TRUE);
		    break;
		case PimEvent::MonthlyEndDay:
		    ((QRadioButton *)monthGroup->find(2))->setChecked(TRUE);
		    break;
		default:
		    ((QRadioButton *)monthGroup->find(0))->setChecked(TRUE);
		    mEvent.setRepeatType(PimEvent::MonthlyDate);
		    break;
	    }
	    fStack->raiseWidget(1);
	    subStack->raiseWidget(2);
	    break;
	case 3:
	    mEvent.setRepeatType(PimEvent::Yearly);
	    fStack->raiseWidget(1);
	    subStack->raiseWidget(0);
	    break;
    }

    refreshLabels();
}

void RepeatEntry::setSubRepeatType(int t)
{
    if (currentID(typeSelector) == 2) {
	switch (t) {
	    case 0:
		mEvent.setRepeatType(PimEvent::MonthlyDate);
		break;
	    case 1:
		mEvent.setRepeatType(PimEvent::MonthlyDay);
		break;
	    default:
		mEvent.setRepeatType(PimEvent::MonthlyEndDay);
		break;
	}
    }
    refreshLabels();
}

void RepeatEntry::setFrequency(int f)
{
    mEvent.setFrequency(f);
    refreshLabels();
}

void RepeatEntry::setRepeatOnWeekDay(int d, bool b)
{
    mEvent.setRepeatOnWeekDay(d, b);
    refreshLabels();
}

void RepeatEntry::setStartOnMonday(bool b)
{
    weekGroup->setStartOnMonday(b);
}

void RepeatEntry::keyPressEvent( QKeyEvent *e )
{
    //qDebug( "key %d (up %d, down %d)", e->key(), Key_Up, Key_Down );
#ifdef QTOPIA_PHONE
    switch ( e->key() ) {
	case Key_Left:
	    nextButton( (QWidget**)typeBtn, 4, TRUE );
	    break;
	case Key_Right:
	    nextButton( (QWidget**)typeBtn, 4 );
	    break;
	default:
	    QDialog::keyPressEvent( e );
	    break;
    }
#else
    QDialog::keyPressEvent( e );
#endif
}

#include "repeatentry.moc"
