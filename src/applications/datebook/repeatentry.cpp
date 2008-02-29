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

#include <qtopia/datepicker.h>
#include <qtopia/calendar.h>

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
	b1->setText(Calendar::nameOfDay(oM));
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

QString RepeatEntry::trOrdinal(int n) const
{
    if ( n == 1 ) return tr("first","eg. first Friday of month");
    else if ( n == 2 ) return tr("second");
    else if ( n == 3 ) return tr("third");
    else if ( n == 4 ) return tr("fourth");
    else if ( n == 5 ) return tr("fifth");
    else return QString::number(n);
}

RepeatEntry::RepeatEntry( bool /* startOnMonday */, const PimEvent &rp,
	QWidget *parent, const char *name, bool modal,
	WFlags fl) : QDialog(parent, name, modal, fl), mEvent(rp)
{
    setCaption(tr("Repeating Event"));
    // RepeatType
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(6);
    layout->setSpacing(6);

    QHBoxLayout *buttonLayout = new QHBoxLayout(layout);
    buttonLayout->setSpacing(0);
    typeSelector = new QButtonGroup(this);
    typeSelector->hide();
    typeSelector->setExclusive(TRUE);
    QPushButton *pb = new QPushButton(tr("Day","Day, not date"), this);
    pb->setToggleButton(TRUE);
    pb->setFocusPolicy(StrongFocus);
    pb->setAutoDefault(FALSE);
    buttonLayout->addWidget(pb);
    typeSelector->insert(pb, 0);
    pb = new QPushButton(tr("Week"), this);
    pb->setToggleButton(TRUE);
    pb->setFocusPolicy(StrongFocus);
    pb->setAutoDefault(FALSE);
    buttonLayout->addWidget(pb);
    typeSelector->insert(pb, 1);
    pb = new QPushButton(tr("Month"), this);
    pb->setToggleButton(TRUE);
    buttonLayout->addWidget(pb);
    pb->setFocusPolicy(StrongFocus);
    pb->setAutoDefault(FALSE);
    typeSelector->insert(pb, 2);
    pb = new QPushButton(tr("Year"), this);
    pb->setToggleButton(TRUE);
    pb->setFocusPolicy(StrongFocus);
    pb->setAutoDefault(FALSE);
    buttonLayout->addWidget(pb);
    typeSelector->insert(pb, 3);

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

    QWidget *freqBox = new QWidget(fStack);
    QVBoxLayout *subLayout = new QVBoxLayout(freqBox);
    QHBoxLayout *subLayout2 = new QHBoxLayout(subLayout);
    subLayout2->setSpacing(6);

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

    weekGroup = new WeekGroup(mEvent.start().date().dayOfWeek(), subStack);
    subStack->addWidget(weekGroup, 1);

    monthGroup = new QVButtonGroup(tr("Repeat On"), subStack);
    monthGroup->setExclusive(TRUE);

    QString strEndWeekDay1 = tr("the last %1.","eg. %1 = Friday");
    strEndWeekDay = tr("the %1 last %2.","eg. %1 last %2 = 2nd last Friday");

    strDate = tr("day %1 of the month.","eg. %1 = 3")
	.arg(mEvent.start().date().day());

    strWeekDay = tr("the %1 %2.","eg. %1 %2 = 2nd Friday")
	.arg(trOrdinal(
		    Calendar::weekInMonth(mEvent.start().date())))
	.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long));

    int fromEndOfWeek = mEvent.start().date().daysInMonth()
	- mEvent.start().date().day();
    fromEndOfWeek = fromEndOfWeek > 0 ? fromEndOfWeek / 7 + 1 : 1;

    strEndWeekDay =
	((fromEndOfWeek > 1)
	    ? strEndWeekDay.arg( trOrdinal(fromEndOfWeek) )
	    : strEndWeekDay1)
	.arg( TimeString::localDayOfWeek( mEvent.start().date(), TimeString::Long ) );

    QRadioButton *r1 = new QRadioButton(strDate, monthGroup);
    r1 = new QRadioButton(strWeekDay, monthGroup);
    r1 = new QRadioButton(strEndWeekDay, monthGroup);



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
    connect(weekGroup, SIGNAL(dayToggled(int, bool)),
	    this, SLOT(setRepeatOnWeekDay(int,bool)));
    connect(monthGroup, SIGNAL(clicked(int)),
	    this, SLOT(setSubRepeatType(int)));
}

RepeatEntry::~RepeatEntry() {}

void RepeatEntry::refreshLabels()
{
    QString type;
    switch (mEvent.repeatType())
    {
	case PimEvent::NoRepeat:
	    descLabel->setText(tr("Don't repeat."));
	    break;
	case PimEvent::Daily:
	    if (mEvent.frequency() == 1)
		descLabel->setText(tr("Repeat every day."));
	    else
		descLabel->setText(tr("Repeat every %1 days.").arg(mEvent.frequency()));
	    type = tr("Every %1 day(s)");
	    break;
	case PimEvent::Weekly:
	    {
		type = tr("Every %1 week(s)");
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
	    }
	    break;
	case PimEvent::MonthlyDate:
	    if (mEvent.frequency() == 1)
		descLabel->setText(
			tr("Repeat every month on day %1 of the month.","eg. %1 = 3")
			.arg(mEvent.start().date().day())
			);
	    else
		descLabel->setText(
			tr("Repeat every %1 months on the %2 of the month.", "eg. %1 = 4, %2 = 3rd")
			.arg(mEvent.frequency())
			.arg(trOrdinal(mEvent.start().date().day()))
			);
	    type = tr("Every %1 month(s)");
	    break;
	case PimEvent::MonthlyDay:
	    if (mEvent.frequency() == 1)
		descLabel->setText(tr("Repeat every month on the %1 %2 of the month.", "eg. %1 %2 = 2nd Friday")
			.arg(trOrdinal(
				Calendar::weekInMonth(mEvent.start().date())))
			.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
			);
	    else
		descLabel->setText(tr("Repeat every %1 months on the %2 %3 of the month.", "eg. %1 = 4, %2 %3 = 2nd Friday")
			.arg(mEvent.frequency()) 
			.arg(trOrdinal(
				Calendar::weekInMonth(mEvent.start().date())))
			.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
			);
	    type = tr("Every %1 month(s)");
	    break;
	case PimEvent::MonthlyEndDay:
	    {
		int fromEndOfWeek = mEvent.start().date().daysInMonth()
		    - mEvent.start().date().day();
		fromEndOfWeek = fromEndOfWeek > 0 ? fromEndOfWeek / 7 + 1 : 1;

		if (mEvent.frequency() == 1) {
		    if (fromEndOfWeek > 1) {
			descLabel->setText(
				tr("Repeat every month on the last %1 of the month.","eg. %1 = Friday")
				.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
				);
		    } else {
			descLabel->setText(
				tr("Repeat every month on %1 last %2 of the month.", "eg. %1 = 2nd, %2 = Friday")
				.arg( trOrdinal(fromEndOfWeek) )
				.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
				);
		    }
		} else {
		    if (fromEndOfWeek > 1) {
			descLabel->setText(
				tr("Repeat every %2 months on the last %1 of the month.","eg. %1 = Friday, %2 = 4")
				.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
				.arg(mEvent.frequency()) 
				);
		    } else {
			descLabel->setText(
				tr("Repeat every %3 months on %1 last %2 of the month.", "eg. %1 = 2nd, %2 = Friday, %3 = 4")
				.arg( trOrdinal(fromEndOfWeek) )
				.arg(TimeString::localDayOfWeek(mEvent.start().date(), TimeString::Long))
				.arg(mEvent.frequency()) 
				);
		    }
		}
		type = tr("Every %1 month(s)");
	    }
	    break;
	case PimEvent::Yearly:
	    if (mEvent.frequency() == 1)
		descLabel->setText(tr("Repeat every year."));
	    else
		descLabel->setText(tr("Repeat every %1 years.").arg(mEvent.frequency()));
	    type = tr("Every %1 year(s)");
	    break;
	default:
	    descLabel->setText("Bug <2316>"); // No tr
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

#include "repeatentry.moc"
