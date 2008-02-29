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

#include "datetimeedit.h"
#include <qvalidator.h>
#include <qapplication.h>
#include <qpopupmenu.h>

class TimeValidator : public QValidator
{
public:
    TimeValidator(QWidget *p, const char *n = 0) : QValidator(p,n)
    {
    }

    State validate(QString &input, int &cpos) const {
	QTime t;
	return evaluate(input, cpos, t);
    }

    State evaluate(QString &input, int &, QTime &result) const
    {
	// <00->23>:<00->59> [AM|PM]
	QString working = input.stripWhiteSpace();

	int i = 0;
	int st = 0; // hour, minute, second, end.
	int count = 0;
	int acc = 0;
	while (i < (int)working.length()) {
	    if (working[i].isSpace()) {
		i++;
		continue;
	    }
	    // expect digits,
	    if (working[i].isDigit()) {
		if (st > 2 || count > 1)
		    return Invalid;
		if (acc)
		    acc *= 10;
		acc += working[i].digitValue();
		count++;
		switch(st) {
		    case 0:
			if (acc > 23)
			    return Intermediate;
			result.setHMS(acc, 0, 0);
			break;
		    case 1:
			if (acc > 59)
			    return Intermediate;
			result.setHMS(result.hour(), acc, 0);
			break;
		    case 2:
			if (acc > 59)
			    return Intermediate;
			result.setHMS(result.hour(), result.minute(), acc);
			break;
		}
	    } else if (working[i] == QChar(':')) {
		if (st > 1 || count == 0)
		    return Intermediate;
		acc = 0;
		st++;
		count = 0;
	    } else if (working[i] == QChar('A')
		    || working[i] == QChar('a')
		    || working[i] == QChar('P')
		    || working[i] == QChar('p')) {
		if (st == 0 && count == 0)
		    return Intermediate;
		acc = 0;
		count = 0;
		st = 3;

		if (working[i] == QChar('p') || working[i] == QChar('P')) {
		    if (result.hour() == 12)
			result.setHMS(12, result.minute(), result.second());
		    else if (result.hour() > 11)
			return Intermediate;
		    else
			result.setHMS(result.hour() + 12, result.minute(), result.second());
		} else {
		    if (result.hour() == 12)
			result.setHMS(0, result.minute(), result.second());
		    else if (result.hour() > 11)
			return Intermediate;
		}
		if (i + 1 == (int)working.length())
		    return Valid;
		if (i + 2 != (int)working.length())
		    return  Intermediate;
		if (working[i+1] == QChar('m')
			|| working[i+1] == QChar('M'))
		    return Valid;
		else
		    return Invalid;
	    } else
		return Invalid;
	    i++;
	}
	return Valid;

    }
};

QPETimeEdit::QPETimeEdit( QWidget *parent, const char *name)
    : QSpinBox(parent, name)
{
    setWrapping(TRUE);

    setMinValue(0);
    setMaxValue(23*60 + 59);
    setLineStep(15);

    connect(this, SIGNAL(valueChanged(int)),
	    this, SLOT(changeTimeUsingValue(int)));

    tv = new TimeValidator(this);

    setValidator(tv);

    TimeString::connectChange(this,SLOT(clockChanged()) );
}

QPETimeEdit::QPETimeEdit( const QTime &dt, QWidget *parent,
	const char *name ) : QSpinBox(parent, name)
{
    setWrapping(TRUE);

    setMinValue(0);
    setMaxValue(23*60 + 59);
    setLineStep(15);

    setTime(dt);

    connect(this, SIGNAL(valueChanged(int)),
	    this, SLOT(changeTimeUsingValue(int)));

    tv = new TimeValidator(this);

    setValidator(tv);
}

void QPETimeEdit::clockChanged()
{
    updateDisplay();
}

QPETimeEdit::~QPETimeEdit()
{
}

// minutes only;
QString QPETimeEdit::mapValueToText(int v)
{
    if (v >= 24*60)
	return QString::null;


    QTime t(v / 60, v % 60);
    return TimeString::localHM(t);
}

int QPETimeEdit::mapTextToValue(bool *ok)
{
    QTime res;
    QString working = text();
    int cpos = 0;
    bool test = (tv->evaluate(working, cpos, res) == QValidator::Valid);

    if (ok)
	*ok = test;

    int h = res.hour();
    int m = res.minute();

    int v = h * 60 + m;

    return v;
}

QTime QPETimeEdit::mapValueToTime(int v) const
{
    QTime tm(v / 60, v % 60,0);
    return tm;
}

void QPETimeEdit::changeTimeUsingValue(int v)
{
    QTime tm = mapValueToTime(v);
    emit valueChanged(tm);
}

void QPETimeEdit::setTime( const QTime &tm )
{
    if (tm == time())
	return;
    setValue(tm.hour() * 60 + tm.minute());
}

QTime QPETimeEdit::time( ) const
{
    QTime tm = mapValueToTime(value());
    return tm;
}

void QPETimeEdit::stepUp()
{
    int tmp = value();
    tmp = tmp - (tmp % 15);
    tmp += 15;
    if (tmp > maxValue())
	tmp = maxValue();
    setValue(tmp);
}

void QPETimeEdit::stepDown()
{
    int tmp = value();
    if (tmp % 15)
	tmp = tmp - (tmp % 15);
    else
	tmp -=15;
    if (tmp < minValue())
	tmp = minValue();
    setValue(tmp);
}



/*=====================================*/


QPEDateEdit::QPEDateEdit( QWidget *parent, const char * name, bool longDate,
			      bool allowNullDate )
    :QPushButton( parent, name ), longFmt(longDate), mAllowNullButton(allowNullDate)
{
    df = TimeString::currentDateFormat();
    noneButton = 0;
    init();
    setLongFormat(longDate);
}

QPEDateEdit::QPEDateEdit( const QDate &dt, QWidget *parent, const char * name, bool longDate,
			      bool allowNullDate )
    :QPushButton( parent, name ), longFmt(longDate), mAllowNullButton(allowNullDate)
{
    currDate = dt;
    noneButton = 0;
    init();
    setLongFormat(longDate);
}

void QPEDateEdit::clockChanged()
{
    df = TimeString::currentDateFormat();
    updateButtonText();
}

void QPEDateEdit::setLongFormat( bool l )
{
    longFmt = l;
    updateButtonText();
}

void QPEDateEdit::init()
{
    QPopupMenu *popup = new QPopupMenu( this );
    monthView = new QPEDatePicker( popup, 0, TRUE);

    // XXX WARNING DO NOT CHANGE THIS UNLESS....
    // you test VERY carefully.  There has been
    // three attempts to allow this to be changed after
    // init, and all of them resulted in a host of wierd bugs.
    // Resulting in having to roll back the change.
    // Watch espcially for:
    //    Focus bugs, whereby the keys don't do what they should
    //                Until TAB is pressed, or you can't get to the
    //                None Button.
    //    Layout bugs, Make sure the button actually hides/shows,
    //		      Both on the first show and later shows.
    //	  Key bugs.  All these should work,
    //		    Left,Right,Up,Down,Space.
    // XXX
    if (mAllowNullButton)  // set to 1 for show, 
	noneButton = new QPushButton(tr("None"), popup);
    // XXX WARNING See above.

    //
    // Geometry information for the QPEDatePicker widget is
    // setup by the QPopupMenu (parent).  However at 176x220,
    // the widget hangs off the side of the screen.  Force the
    // widget to be bounded by the desktop width.
    //
    monthView->setMaximumWidth(qApp->desktop()->width());

    popup->insertItem( monthView );

    if (mAllowNullButton) {
	popup->insertItem( noneButton );
	connect(noneButton, SIGNAL( clicked() ), this, SLOT(setNull()));
    }

    connect( monthView, SIGNAL( dateClicked( const QDate &) ),
	    this, SLOT( setDate( const QDate &) ) );
    connect( monthView, SIGNAL( dateClicked( const QDate &) ),
	    this, SIGNAL( valueChanged( const QDate &) ) );
    TimeString::connectChange(this,SLOT(clockChanged()));
    setPopup( popup );
    popup->setFocusPolicy(NoFocus);
    setDate( QDate::currentDate() );
    setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed) );
}

bool QPEDateEdit::allowNullDate() const
{
    return mAllowNullButton;
}

void QPEDateEdit::setWeekStartsMonday( bool b )
{
    weekStartsMonday = b;
    monthView->setWeekStartsMonday( weekStartsMonday );
}

void QPEDateEdit::setNull()
{
    popup()->close();
    setDate( QDate() );
    emit valueChanged( QDate() );
}

void QPEDateEdit::updateButtonText()
{
    if ( currDate.isValid() ) {
#if defined(_WS_QWS_)
	setText( longFmt ? TimeString::longDateString(currDate,df) : TimeString::dateString(currDate,df) );
#else
	//setText( longFmt ? currDate.toString( "ddd MMMM d yy" ) : currDate.toString("dd/MM/yyyy"));
	setText( longFmt ? currDate.toString( "ddd MMMM d yy" ) : currDate.toString(Qt::LocalDate));
#endif
	monthView->setDate( currDate.year(), currDate.month(), currDate.day() );
    }
    else {
	setText( tr("None") );
    }
}

QDate QPEDateEdit::date() const
{
    return currDate;
}

void QPEDateEdit::setDate( const QDate &d )
{
    if ( d != currDate ) {
	currDate = d;
	updateButtonText();
    }
}

void QPEDateEdit::setDateFormat( DateFormat f )
{
    df = f;
    updateButtonText();
}

/*=====================================*/


QPEDateTimeEdit::QPEDateTimeEdit( QWidget *parent, const char *name)
    : QHBox(parent, name)
{
    de = new QPEDateEdit(this, "date"); // No tr
    te = new QPETimeEdit(this, "time"); // No tr
    connect(de, SIGNAL(valueChanged(const QDate &)),
	    this, SLOT(setDate( const QDate &)));
    connect(te, SIGNAL(valueChanged(const QTime &)),
	    this, SLOT(setTime( const QTime &)));
}

QPEDateTimeEdit::QPEDateTimeEdit( const QDateTime &dt, QWidget *parent,
	const char *name ) : QHBox(parent, name)
{
    de = new QPEDateEdit(dt.date(), this, "date"); // No tr
    te = new QPETimeEdit(dt.time(), this, "time"); // No tr
    connect(de, SIGNAL(valueChanged(const QDate &)),
	    this, SLOT(setDate( const QDate &)));
    connect(te, SIGNAL(valueChanged(const QTime &)),
	    this, SLOT(setTime( const QTime &)));
}

QPEDateTimeEdit::~QPEDateTimeEdit() {}

void QPEDateTimeEdit::setDate( const QDate &dt )
{
    de->setDate(dt);
    emit valueChanged(dateTime());
}

void QPEDateTimeEdit::setTime( const QTime &dt )
{
    te->setTime(dt);
    emit valueChanged(dateTime());
}

void QPEDateTimeEdit::setDateTime( const QDateTime &dt )
{
    de->setDate(dt.date());
    te->setTime(dt.time());
    emit valueChanged(dateTime());
}

QDate QPEDateTimeEdit::date() const
{
    return de->date();
}

QTime QPEDateTimeEdit::time() const
{
    return te->time();
}

QDateTime QPEDateTimeEdit::dateTime() const
{
    return QDateTime(de->date(), te->time());
}

void QPEDateTimeEdit::setDateEnabled(bool b)
{
    de->setEnabled(b);
}

void QPEDateTimeEdit::setTimeEnabled(bool b)
{
    te->setEnabled(b);
}

bool QPEDateTimeEdit::dateIsEnabled() const
{
    return de->isEnabled();
}

bool QPEDateTimeEdit::timeIsEnabled() const
{
    return te->isEnabled();
}
