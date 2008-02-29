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
	    if (working[i].isSpace() && (count == 0 || count == 2)) {
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
    ampm = TRUE;
    setWrapping(TRUE);

    setMinValue(0);
    setMaxValue(23*60 + 59);
    setLineStep(15);

    connect(this, SIGNAL(valueChanged(int)),
	    this, SLOT(changeTimeUsingValue(int)));

    tv = new TimeValidator(this);

    setValidator(tv);
}

QPETimeEdit::QPETimeEdit( const QTime &dt, QWidget *parent,
	const char *name ) : QSpinBox(parent, name)
{
    ampm = TRUE;
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

QPETimeEdit::~QPETimeEdit()
{
}

// minutes only;
QString QPETimeEdit::mapValueToText(int v)
{
    if (v >= 24*60)
	return QString::null;


    QTime t(v / 60, v % 60);
    return TimeString::timeString(t, ampm);
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

void QPETimeEdit::setClock(bool b)
{
    if (ampm == b)
	return;
    ampm = b;
    updateDisplay();
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

//=======================================

QPEDateEdit::QPEDateEdit( QWidget *parent, const char *name)
    : QPEDateButton(parent, name)
{
    connect(this, SIGNAL(dateSelected(const QDate &)),
	    this, SLOT(selectDate(const QDate &)));
}

QPEDateEdit::QPEDateEdit( const QDate &dt, bool longDate, QWidget *parent,
	const char *name)
    : QPEDateButton( parent, name, longDate )
{
    setDate(dt);
    connect(this, SIGNAL(dateSelected(int, int, int)),
	    this, SLOT(selectDate(int,int,int)));
}

QPEDateEdit::~QPEDateEdit(){}

void QPEDateEdit::selectDate(const QDate &dt)
{
    emit valueChanged(dt);
}

void QPEDateEdit::setDate( const QDate &dt )
{
    if (dt == date())
	return;
    QPEDateButton::setDate(dt.year(), dt.month(), dt.day());
}

//========================================================

QPEDateTimeEdit::QPEDateTimeEdit( QWidget *parent, const char *name)
    : QHBox(parent, name)
{
    de = new QPEDateEdit(this, "date");
    te = new QPETimeEdit(this, "time");
    connect(de, SIGNAL(valueChanged(const QDate &)),
	    this, SLOT(setDate( const QDate &)));
    connect(te, SIGNAL(valueChanged(const QTime &)),
	    this, SLOT(setTime( const QTime &)));
}

QPEDateTimeEdit::QPEDateTimeEdit( const QDateTime &dt, QWidget *parent,
	const char *name ) : QHBox(parent, name)
{
    de = new QPEDateEdit(dt.date(), FALSE, this, "date");
    te = new QPETimeEdit(dt.time(), this, "time");
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

void QPEDateTimeEdit::setClock(bool b)
{
    te->setClock(b);
}
