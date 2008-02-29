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

#include "repeatentry.h"

#include "datepicker.h"
#include <qtopia/pim/calendar.h>

#include <qtopia/qpeapplication.h>

#include <qtabbar.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qhbuttongroup.h>
#include <qvbuttongroup.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qwidgetstack.h>
#include <qtoolbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qsimplerichtext.h>
#include <qpainter.h>

class RichLabel : public QLabel
{
public:
    RichLabel(QWidget *p) : QLabel(p) { }
protected: 
    void drawContents(QPainter *p)
    {
	QSimpleRichText rt(text(), p->font());
	rt.setWidth(width());
	rt.draw(p, 0,0, QRegion(geometry()), palette());
    }
};


class WeekGroup : public QButtonGroup
{
    Q_OBJECT

public:
    WeekGroup(int oM, QWidget *parent = 0);

    ~WeekGroup() {}

    void toggleDay(int, bool);

public slots:
    void setStartOnMonday(bool b);

signals:
    void dayToggled(int, bool);

private slots:
    void setDay(int);

private:
    //int firstDayOfWeek;
    QToolButton *first;
    QToolButton *last;
};

WeekGroup::WeekGroup(int oM, QWidget *parent)
    : QButtonGroup(QString("Repeat On"), parent)
{

    setColumnLayout(0, Qt::Vertical);
    layout()->setSpacing(0);
    layout()->setMargin(0);
    QHBoxLayout *fe = new QHBoxLayout(layout());
    fe->setAlignment( Qt::AlignTop );
    fe->setSpacing(0);
    fe->setMargin(6);

    setExclusive(FALSE);
    layout()->setSpacing(0);

    /*
    first = new QToolButton(this);
    first->setText(tr("Sun"));
    first->setToggleButton(TRUE);
    fe->addWidget(first);
    */

    QToolButton *b1;
    for (int i = 0; i < 7; i++) {
	b1 = new QToolButton(this);
	b1->setToggleButton(TRUE);
	if (!i) {
	    b1->setEnabled(FALSE);
	    //b1->setOn(TRUE);
	}
	b1->setText(PimCalendar::nameOfDay(oM));
	insert(b1, oM);
	fe->addWidget(b1);
	oM++;
	if (oM > 7) 
	    oM = 1;
    }

   /* 
    QToolButton *b1 = new QToolButton(this);
    b1->setText(tr("Mon"));
    b1->setToggleButton(TRUE);
    fe->addWidget(b1);
    b1 = new QToolButton(this);
    b1->setText(tr("Tue"));
    b1->setToggleButton(TRUE);
    fe->addWidget(b1);
    b1 = new QToolButton(this);
    b1->setText(tr("Wed"));
    b1->setToggleButton(TRUE);
    fe->addWidget(b1);
    b1 = new QToolButton(this);
    b1->setText(tr("Thu"));
    b1->setToggleButton(TRUE);
    fe->addWidget(b1);
    b1 = new QToolButton(this);
    b1->setText(tr("Fri"));
    b1->setToggleButton(TRUE);
    fe->addWidget(b1);
    b1 = new QToolButton(this);
    b1->setText(tr("Sat"));
    b1->setToggleButton(TRUE);
    fe->addWidget(b1);

    last = new QToolButton(this);
    last->setText(tr("Sun"));
    last->setToggleButton(TRUE);
    fe->addWidget(last);

    if (onMonday)
	first->hide();
    else
	last->hide();
*/
    connect(this, SIGNAL(clicked(int)), this, SLOT(setDay(int)));
}

void WeekGroup::setStartOnMonday(bool b)
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

RepeatEntry::RepeatEntry( bool startOnMonday, const PimEvent &rp, 
	QWidget *parent, const char *name, bool modal, 
	WFlags fl) : QDialog(parent, name, modal, fl), mEvent(rp)
{

    // RepeatType
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(6);
    layout->setSpacing(6);

    typeSelector = new QTabBar(this);
    typeSelector->addTab(new QTab(tr("None")));
    typeSelector->addTab(new QTab(tr("Day")));
    typeSelector->addTab(new QTab(tr("Week")));
    typeSelector->addTab(new QTab(tr("Month")));
    typeSelector->addTab(new QTab(tr("Year")));

    switch(mEvent.repeatType()) {
	case PimEvent::NoRepeat:
	    typeSelector->setCurrentTab(0);
	    break;
	case PimEvent::Daily:
	    typeSelector->setCurrentTab(1);
	    break;
	case PimEvent::Weekly:
	    typeSelector->setCurrentTab(2);
	    break;
	case PimEvent::MonthlyDate:
	case PimEvent::MonthlyDay:
	case PimEvent::MonthlyEndDay:
	    typeSelector->setCurrentTab(3);
	    break;
	case PimEvent::Yearly:
	    typeSelector->setCurrentTab(4);
	    break;
    }

    layout->addWidget(typeSelector);

    // Frequency and end date
    fStack = new QWidgetStack(this);
    fStack->addWidget(new QWidget(fStack), 0);

    QWidget *freqBox = new QWidget(fStack);
    QVBoxLayout *subLayout = new QVBoxLayout(freqBox);
    QHBoxLayout *subLayout2 = new QHBoxLayout(subLayout);
    subLayout2->setSpacing(6);
    QHBoxLayout *subLayout3 = new QHBoxLayout(subLayout);
    subLayout3->setSpacing(6);

    QLabel *l = new QLabel(tr("Every:"), freqBox);
    subLayout2->addWidget(l);
    freqSelector = new QSpinBox(freqBox);
    freqSelector->setMinValue(1);
    subLayout2->addWidget(freqSelector);
    typeLabel = new QLabel(freqBox);
    subLayout2->addWidget(typeLabel);

    freqSelector->setValue(mEvent.frequency());

    l = new QLabel(tr("End On:"), freqBox);
    subLayout3->addWidget(l);
    endSelect = new QPEDateButton(freqBox);
    subLayout3->addWidget(endSelect);
    hasEndCheck = new QCheckBox(tr("No End Date"), freqBox);
    subLayout3->addWidget(hasEndCheck);

    freqSelector->setValue(mEvent.frequency());

    endSelect->setDate(mEvent.repeatTill());
    if (mEvent.hasRepeat()) {
	endSelect->setEnabled(!mEvent.repeatForever());
	hasEndCheck->setChecked(mEvent.repeatForever());
    } else {
	endSelect->setEnabled(FALSE);
	hasEndCheck->setChecked(TRUE);
    }

    fStack->addWidget(freqBox, 1);

    layout->addWidget(fStack);

    // Now for the day select/ month type select part
    subStack = new QWidgetStack(this);
    subStack->addWidget(new QWidget(subStack), 0);

    weekGroup = new WeekGroup(mEvent.start().date().dayOfWeek(), subStack);
    subStack->addWidget(weekGroup, 1);

    monthGroup = new QVButtonGroup(tr("Repeat On"), subStack);
    monthGroup->setExclusive(TRUE);
    /*
    QToolButton *b1 = new QToolButton(monthGroup);
    b1->setText(tr("Day"));
    b1->setToggleButton(TRUE);
    b1 = new QToolButton(monthGroup);
    b1->setText(tr("Date"));
    b1->setToggleButton(TRUE);
    */

    strDate = tr("the %1 of the month.");
    strWeekDay = tr("the %1 %2 of the month.");
    strEndWeekDay = tr("the %1last %2 of the month.");

    strDate = strDate
	.arg(PimCalendar::ordinalNumber(mEvent.start().date().day()));
    strWeekDay = strWeekDay
	.arg(PimCalendar::ordinalNumber(
		    PimCalendar::weekOfDate(mEvent.start().date())))
	.arg(PimCalendar::nameOfDay(mEvent.start().date(), TRUE));

    int fromEndOfWeek = mEvent.start().date().daysInMonth() 
	- mEvent.start().date().day();
    fromEndOfWeek = fromEndOfWeek > 0 ? fromEndOfWeek / 7 + 1 : 1;

    strEndWeekDay = strEndWeekDay
	.arg( fromEndOfWeek > 1 ? PimCalendar::ordinalNumber(fromEndOfWeek) + " " 
		: QString(""))
	.arg( PimCalendar::nameOfDay( mEvent.start().date(), TRUE ) );

    QRadioButton *r1 = new QRadioButton(strDate, monthGroup);
    r1 = new QRadioButton(strWeekDay, monthGroup);
    // XXX uncomment next line to allow end of month recurrance
    //r1 = new QRadioButton(strEndWeekDay, monthGroup);



    subStack->addWidget(monthGroup, 2);

    layout->addWidget(subStack);

    //QSpacerItem *si = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    //layout->addItem(si);
    // and last but not least, the label.. later.
    //QFrame
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

    // pretend we just changed repeat types.
    setRepeatType(typeSelector->currentTab());

    // Connection time.
    connect(typeSelector, SIGNAL(selected(int)), this, SLOT(setRepeatType(int)));
    connect(freqSelector, SIGNAL(valueChanged(int)), 
	    this, SLOT(setFrequency(int)));
    connect(hasEndCheck, SIGNAL(toggled(bool)), 
	    this, SLOT(setHasEndDate(bool)));
    connect(endSelect, SIGNAL(dateSelected(int,int,int)), 
	    this, SLOT(setEndDate(int,int,int)));
    connect(weekGroup, SIGNAL(dayToggled(int, bool)), 
	    this, SLOT(setRepeatOnWeekDay(int,bool)));
    connect(monthGroup, SIGNAL(clicked(int)), 
	    this, SLOT(setSubRepeatType(int)));
}

RepeatEntry::~RepeatEntry() {}

void RepeatEntry::refreshLabels()
{
    switch (mEvent.repeatType())
    {
	case PimEvent::NoRepeat:
	    descLabel->setText(tr("Don't repeat."));
	    break;
	case PimEvent::Daily:
	    descLabel->setText(tr("Repeat every %1 days.").arg(mEvent.frequency()));
	    typeLabel->setText(tr("day(s)"));
	    break;
	case PimEvent::Weekly:
	    {
		QString buf = tr("Repeat every %1 weeks on ");
		QString lastday;
		bool multi = FALSE;
		int dow = mEvent.start().date().dayOfWeek();
		for (int i = 0; i < 7; i++) {
		    if (mEvent.repeatOnWeekDay(dow)) {
			if (!lastday.isEmpty()) {
			    if (multi)
				buf += ", ";
			    buf += lastday;
			    multi = TRUE;
			}
			lastday = PimCalendar::nameOfDay(dow, TRUE);
		    }
		    dow++;
		    if (dow > 7) dow = 1;
		}
		if (multi)
		    buf += tr(" and ");
		buf += lastday + ".";
		typeLabel->setText(tr("week(s)"));
		descLabel->setText(buf.arg(mEvent.frequency()));
	    }
	    break;
	case PimEvent::MonthlyDate:
	    descLabel->setText(tr("Repeat every %1 months on ")
		    .arg(mEvent.frequency()) + strDate);
	    typeLabel->setText(tr("month(s)"));
	    break;
	case PimEvent::MonthlyDay:
	    descLabel->setText(tr("Repeat every %1 months on ")
		    .arg(mEvent.frequency()) + strWeekDay);
	    typeLabel->setText(tr("month(s)"));
	    break;
	case PimEvent::MonthlyEndDay:
	    descLabel->setText(tr("Repeat every %1 months on ")
		    .arg(mEvent.frequency()) + strEndWeekDay);
	    typeLabel->setText(tr("month(s)"));
	    break;
	case PimEvent::Yearly:
	    descLabel->setText(tr("Repeat every %1 years.").arg(mEvent.frequency()));
	    typeLabel->setText(tr("year(s)"));
	    break;
	default:
	    descLabel->setText("Bug <2316>");
	    typeLabel->setText("");
    }
}

PimEvent RepeatEntry::event() const
{
    return mEvent;
}

void RepeatEntry::setRepeatType(int t)
{
    int i;
    bool setRForever = (t != 0 && !mEvent.hasRepeat());
    switch (t) {
	case 0:
	    mEvent.setRepeatType(PimEvent::NoRepeat);
	    fStack->raiseWidget(0);
	    subStack->raiseWidget(0);
	    break;
	case 1:
	    mEvent.setRepeatType(PimEvent::Daily);
	    fStack->raiseWidget(1);
	    subStack->raiseWidget(0);
	    break;
	case 2:
	    mEvent.setRepeatType(PimEvent::Weekly);
	    fStack->raiseWidget(1);
	    subStack->raiseWidget(1);
	    for(i = 1; i <= 7; i++)
		weekGroup->toggleDay(i, mEvent.repeatOnWeekDay(i));
	    break;
	case 3:
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
	case 4:
	    mEvent.setRepeatType(PimEvent::Yearly);
	    fStack->raiseWidget(1);
	    subStack->raiseWidget(0);
	    break;
    }
    if (setRForever) {
	// default from a non-repeater to a repeater is to set repeat
	// foerever to true
	endSelect->setEnabled(FALSE);
	hasEndCheck->setChecked(TRUE);
	mEvent.setRepeatForever(TRUE);
    }

    refreshLabels();
}

void RepeatEntry::setSubRepeatType(int t)
{
    if (typeSelector->currentTab() == 3) {
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

void RepeatEntry::setEndDate( const QDate &d )
{
    mEvent.setRepeatTill(d);
    refreshLabels();
}

void RepeatEntry::setEndDate( int y, int m, int d )
{
    setEndDate(QDate(y,m,d));
}

void RepeatEntry::setHasEndDate(bool b)
{
    mEvent.setRepeatForever(b);
    endSelect->setEnabled(!b);
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
    endSelect->setWeekStartsMonday(b);
}

#include "repeatentry.moc"
