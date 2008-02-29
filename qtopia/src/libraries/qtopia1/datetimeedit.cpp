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

#include "datetimeedit.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/timestring.h>
#include <qtopia/config.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextbar.h>
#include <qtopia/global.h>
#endif

#include <qvalidator.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qlayout.h>

#ifdef QTOPIA_PHONE
extern bool mousePreferred; // can't call Global::mousePreferred in libqtopia2 from libqtopia
#endif

#ifdef Q_WS_QWS
extern Q_EXPORT QRect qt_maxWindowRect;
#endif

class QPEDatePickerPopup : public QFrame
{
    Q_OBJECT

public:
    QPEDatePickerPopup( bool showNullButton, QWidget *parent, const char *name = 0, int wflags = 0);

    QDate selectedDate() const;
signals:
    void dateClicked(const QDate &);

public slots:
    void setDate(const QDate &);
    void setWeekStartsMonday(bool);
    void popup(); // populates widget.
    
protected:
    void closeEvent( QCloseEvent *e );
    void keyPressEvent( QKeyEvent *e );

private slots:
    void passOnNullDate();

private:
    void populate();

    bool mShowNull;
    QPEDatePicker *monthView;
    QPushButton *nullButton;

    bool wsm;
    QDate mdt;
};

class QPEDateEditPrivate
{
public:
    QPEDateEditPrivate() : popup(0) {}

    QPEDatePickerPopup *popup;
};

static bool fromString(const QString& input, QTime& result)
{
    int h,m,s;
    h=m=s=0;
    int i = 0;
    int st = 0; // hour, minute, second, end.
    int count = 0;
    int acc = 0;
    while (i < (int)input.length()) {
	if (input[i].isSpace()) {
	    i++;
	    continue;
	}
	// expect digits,
	if (input[i].isDigit()) {
	    if (st > 2 || count > 1)
		return FALSE;
	    if (acc)
		acc *= 10;
	    acc += input[i].digitValue();
	    count++;
	    switch(st) {
		case 0:
		    if (acc > 23)
			return FALSE;
		    h = acc;
		    break;
		case 1:
		    if (acc > 59)
			return FALSE;
		    m = acc;
		    break;
		case 2:
		    if (acc > 59)
			return FALSE;
		    s = acc;
		    break;
	    }
	} else if (input[i] == QChar(':')) {
	    if (st > 1 || count == 0)
		return FALSE;
	    acc = 0;
	    st++;
	    count = 0;
	} else if (input[i] == QChar('A')
		|| input[i] == QChar('a')
		|| input[i] == QChar('P')
		|| input[i] == QChar('p')) {
	    if (st == 0 && count == 0)
		return FALSE;
	    acc = 0;
	    count = 0;
	    st = 3;

	    if (input[i] == QChar('p') || input[i] == QChar('P')) {
		if (h == 12)
		    ;
		else if (h > 11)
		    return FALSE;
		else
		    h += 12;
	    } else {
		if (h == 12)
		    h = 0;
		else if (h > 11)
		    return FALSE;
	    }
	    if (i + 1 == (int)input.length())
		break;
	    if (i + 2 != (int)input.length())
		return FALSE;
	    if (input[i+1] == QChar('m')
		    || input[i+1] == QChar('M'))
		break;
	    else
		return FALSE;
	} else
	    return FALSE;
	i++;
    }
    result.setHMS(h,m,s);
    return TRUE;
}

class TimeValidator : public QValidator
{
public:
    TimeValidator(QWidget *p, const char *n = 0) : QValidator(p,n)
    {
    }

    State validate(QString &input, int &curs) const
    {
	QTime t;

	// <00->23>:<00->59>[AM|PM]

	QString k;
	int digs=0;
	int ncurs=0;
	for (int i=0; i<(int)input.length(); i++) {
	    char c=input[i].latin1();
	    if ( c>='0' && c<='9' ) {
		QString s; s+=c;
		if ( i > 3 ) {
#ifdef QTOPIA_PHONE
		    if ( digs == 2 ) {
			digs = 0;
			if ( c == '2' )
			    s = " AM";
			else if ( c == '7' )
			    s = " PM";
			else
			    s = QString::null;
		    }
#endif
		} else if ( digs == 2 ) {
		    digs = 0;
		    k += ':';
		    if ( i<curs ) ncurs++;
		}
		k += s;
		digs+=s.length();
		if ( i<curs ) ncurs+=s.length();
	    } else if ( c == ' ' ) {
		digs = 0;
		k += c;
		if ( i<curs ) ncurs++;
	    } else if ( c == 'a' || c == 'A' || c == 'p' || c == 'P'
		    || c == 'm' || c == 'M' ) {
		digs = 0;
		k += c;
		if ( i<curs ) ncurs++;
	    } else if ( c == ':' || digs==2 ) {
		digs = 0;
		k += ':';
		if ( i<curs ) ncurs++;
	    }
	}
	input = k;
	curs = ncurs;
	return Acceptable;
    }
};


/*!
  \class QPETimeEdit datetimeedit.h
  \brief The QPETimeEdit class provides a compact widget for selecting a time.

  QPETimeEdit extends QSpinBox to validate times as they are entered.
  It handles 12 hour and 24 hour time formats.

  First availability: Qtopia 1.6

  \ingroup qtopiaemb
*/

/*!
  \fn QPETimeEdit::valueChanged( const QTime &t );

  This signal is emitted when the time is changed to \a t.
*/

/*!
  Constructs a QPETimeEdit with the time set to 00:00.
    The \a parent and \a name parameters are the standard Qt parent parameters.
*/
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
    QPEApplication::setInputMethodHint(this,QPEApplication::Number);

    setValidator(tv);

    TimeString::connectChange(this,SLOT(clockChanged()) );
}

/*!
  Constructs a QPETimeEdit with the time set to \a dt.
    The \a parent and \a name parameters are the standard Qt parent parameters.
*/
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
    QPEApplication::setInputMethodHint(this,QPEApplication::Number);

    setValidator(tv);
}

/*!
  \internal
*/
void QPETimeEdit::clockChanged()
{
    updateDisplay();
}

/*!
  Destructs QPETimeEdit.
*/
QPETimeEdit::~QPETimeEdit()
{
}

/*!
  \reimp

  Reimplemented to validate the time entered.
*/
// minutes only;
QString QPETimeEdit::mapValueToText(int v)
{
    if (v >= 24*60)
	return QString::null;


    QTime t(v / 60, v % 60);
    return TimeString::localHM(t);
}

/*!
  \reimp

  Reimplemented to validate the time entered.
*/
int QPETimeEdit::mapTextToValue(bool *ok)
{
    QTime res;
    bool test = fromString(text(),res);

    if (ok)
	*ok = test;

    int h = res.hour();
    int m = res.minute();

    int v = h * 60 + m;

    return v;
}

/*!
  \internal
*/
QTime QPETimeEdit::mapValueToTime(int v) const
{
    QTime tm(v / 60, v % 60,0);
    return tm;
}

/*!
  \internal
*/
void QPETimeEdit::changeTimeUsingValue(int v)
{
    QTime tm = mapValueToTime(v);
    emit valueChanged(tm);
}

/*!
  Sets displayed time to \a tm.

  \sa time()
*/
void QPETimeEdit::setTime( const QTime &tm )
{
    if (tm == time())
	return;
    setValue(tm.hour() * 60 + tm.minute());
}

/*!
  Returns the time selected.

  \sa setTime()
*/
QTime QPETimeEdit::time( ) const
{
    QTime tm = mapValueToTime(value());
    return tm;
}

/*!
  Increases the time by 15 minutes.
*/
void QPETimeEdit::stepUp()
{
    int tmp = value();
    tmp = tmp - (tmp % 15);
    tmp += 15;
    if (tmp > maxValue()) {
	if (parent() && qstrcmp(parent()->className(), "QPEDateTimeEdit") == 0) {
	    QPEDateTimeEdit *dte = (QPEDateTimeEdit *)parent();
	    dte->blockSignals(TRUE);
	    dte->setDate(dte->date().addDays(1));
	    dte->blockSignals(FALSE);
	}
	tmp = 0;
    }
    setValue(tmp);
}

/*!
  Decreases the time by 15 minutes.
*/
void QPETimeEdit::stepDown()
{
    int tmp = value();
    if (tmp % 15)
	tmp = tmp - (tmp % 15);
    else
	tmp -=15;
    if (tmp < minValue()) {
	if (parent() && qstrcmp(parent()->className(), "QPEDateTimeEdit") == 0) {
	    QPEDateTimeEdit *dte = (QPEDateTimeEdit *)parent();
	    dte->blockSignals(TRUE);
	    dte->setDate(dte->date().addDays(-1));
	    dte->blockSignals(FALSE);
	}
	tmp = 23*60+45;
    }
    setValue(tmp);
}



/*=====================================*/

/*!
  \class QPEDateEdit datetimeedit.h
  \brief The QPEDateEdit class provides a compact widget for selecting
  a date.

  QPEDateEdit displays a QPushButton with the selected date displayed.
  When clicked, a QPEDatePicker is popped up and a new date can be
  selected.

  The button can display the date in either long or short format.

  It is also possible to allow no to date selected.

  \ingroup qtopiaemb
*/

/*!
  \fn QPEDateEdit::valueChanged( const QDate &d );

  This signal is emitted when the date is changed to \a d.
*/

/*!
  \fn bool QPEDateEdit::longFormat() const;

  Returns TRUE if dates are displayed in long format.

  \sa setLongFormat(), setDateFormat()
*/

/*!
  Constructs QPEDateEdit with the current date selected.

  If \a longDate is TRUE the push button will show the selected date in
  long format.

  If \a allowNullDate is TRUE it is possible to select no date.

    The \a parent and \a name parameters are the standard Qt parent parameters.
*/
QPEDateEdit::QPEDateEdit( QWidget *parent, const char * name, bool longDate,
			      bool allowNullDate )
    :QPushButton( parent, name ), longFmt(longDate), mAllowNullButton(allowNullDate), d(0)
{
    df = TimeString::currentDateFormat();
    noneButton = 0;
    init();
    setLongFormat(longDate);
    connect( qApp, SIGNAL(weekChanged(bool)), this, SLOT(setWeekStartsMonday(bool)) );
}


/*!
  Constructs QPEDateEdit with the date \a dt.

  If \a longDate is TRUE the push button will show the selected date in
  long format.

  If \a allowNullDate is TRUE it is possible to select no date.

    The \a parent and \a name parameters are the standard Qt parent parameters.
*/
QPEDateEdit::QPEDateEdit( const QDate &dt, QWidget *parent, const char * name, bool longDate,
			      bool allowNullDate )
    :QPushButton( parent, name ), longFmt(longDate), mAllowNullButton(allowNullDate), d(0)
{
    currDate = dt;
    noneButton = 0;
    init();
    setLongFormat(longDate);
}

/*!
  \internal
*/
void QPEDateEdit::clockChanged()
{
    df = TimeString::currentDateFormat();
    updateButtonText();
}

/*!
  If \a l is TRUE, display the date in long format.

  \sa longFormat(), setDateFormat()
*/
void QPEDateEdit::setLongFormat( bool l )
{
    longFmt = l;
    updateButtonText();
}

//
// Ensure the date is changed when the popup is hidden.  This catches
// changes of year in QPEDatePicker without altering it's behaviour
// (ie. date displayed in the pushbutton only changes when the monthview
// is hidden).
//
void QPEDateEdit::hidingMonthView(void)
{
    setDate(d->popup->selectedDate());
}

QPEDatePickerPopup::QPEDatePickerPopup( bool showNullButton, QWidget *parent,
	const char *name, int wflags) : QFrame(parent, name, WType_Popup
#ifdef QTOPIA_PHONE
	| WStyle_NormalBorder
#endif
	| wflags),
	mShowNull(showNullButton), monthView(0), nullButton(0), wsm(FALSE)
{
    setFocusPolicy(NoFocus);
#ifdef QTOPIA_PHONE
    setCaption(tr("Select Date"));
    ContextBar::setLabel(this, Key_Back, ContextBar::Cancel);
    setFrameStyle(NoFrame);
#else
    setFrameStyle(StyledPanel | Raised);
#endif
}

QDate QPEDatePickerPopup::selectedDate() const
{
    if (monthView)
	return monthView->selectedDate();
    return mdt;
}

void QPEDatePickerPopup::setDate(const QDate &dt)
{
    mdt = dt;
    if (monthView)
	monthView->setDate(dt);
}

void QPEDatePickerPopup::passOnNullDate()
{
    emit dateClicked(QDate());
}

void QPEDatePickerPopup::setWeekStartsMonday(bool b)
{
    wsm = b;
    if (monthView)
	monthView->setWeekStartsMonday(b);
}

void QPEDatePickerPopup::keyPressEvent( QKeyEvent *e )
{
#ifdef QTOPIA_PHONE
    if (e->key() == Key_Back || e->key() == Key_No) {
	e->accept();
	close();
    }
#else
    Q_UNUSED(*e);
#endif
}

void QPEDatePickerPopup::closeEvent( QCloseEvent *e )
{
    e->accept();

    if (!parentWidget())
	return;

    // remember that we (as a popup) might recieve the mouse release
    // event instead of the popupParent. This is due to the fact that
    // the popupParent popped us up in its mousePressEvent handler. To
    // avoid the button remaining in pressed state we simply send a
    // faked mouse button release event to it.
    QMouseEvent me( QEvent::MouseButtonRelease, QPoint(0,0), QPoint(0,0), QMouseEvent::LeftButton, QMouseEvent::NoButton);
    QApplication::sendEvent( parentWidget(), &me );
}

void QPEDatePickerPopup::popup()
{
    populate();
#ifndef Q_WS_QWS
    QRect qt_maxWindowRect = qApp->desktop()->geometry();
#endif
#ifdef QTOPIA_PHONE
    showMaximized();
    monthView->setFocus();
    if( !mousePreferred )
	monthView->setModalEditing(TRUE);
#else
    // base of parents geometry.
    QWidget *p = parentWidget();
    if (p) {
	//want to show just above, or just below,
	//want to show fully inside window rect.
	// which is greater width, above or below.
	QRect pg(p->mapToGlobal(QPoint(0,0)), p->size());
	QRect geo;
	geo.setSize(QSize(sizeHint().width()+2*frameWidth(), sizeHint().height()+2*frameWidth()));
	if (pg.y() > qt_maxWindowRect.height() - (pg.y() + pg.height())) {
	    // above,
	    geo.moveTopLeft(
		    QPoint(QMIN(pg.x(), qt_maxWindowRect.width() - geo.width()),
	    QMAX(qt_maxWindowRect.y(), pg.y() - geo.height())));
	} else {
//	    // below.
	    geo.moveTopLeft(QPoint(QMIN(pg.x(), qt_maxWindowRect.width() - geo.width()),
		    QMIN(qt_maxWindowRect.height() - geo.height(), pg.bottom() )));
	}
	setGeometry(geo);
    } else {
	// um.... full screen?
	setGeometry(qt_maxWindowRect);
    }
#endif
    show();
}

void QPEDatePickerPopup::populate()
{
    if (!monthView) {
	QVBoxLayout *vbl = new QVBoxLayout(this);
#ifdef QTOPIA_PHONE
	vbl->setMargin(0);
#else
	vbl->setMargin(frameWidth());
#endif
	monthView = new QPEDatePicker(this);
	connect(monthView, SIGNAL(dateClicked(const QDate &)), this, SIGNAL(dateClicked(const QDate &)));
	connect(monthView, SIGNAL(dateClicked(const QDate &)), this, SLOT(close()));
	vbl->addWidget(monthView);

	if (mdt.isValid())
	    monthView->setDate(mdt);
	else
	    monthView->setDate(QDate::currentDate());

	monthView->setWeekStartsMonday(wsm);

	if (mShowNull) {
	    nullButton = new QPushButton(tr("None", "no date selected"), this);
	    connect(nullButton, SIGNAL( clicked() ), this, SLOT(passOnNullDate()));
	    connect(nullButton, SIGNAL( clicked() ), this, SLOT(close()));
	    vbl->addWidget(nullButton);
	}
    }
}
void QPEDateEdit::init()
{
    d = new QPEDateEditPrivate();
    d->popup = new QPEDatePickerPopup( mAllowNullButton, this );
    connect(this, SIGNAL(pressed()), d->popup, SLOT(popup()));

    connect( d->popup, SIGNAL( dateClicked(const QDate&) ),
	    this, SLOT( setDate(const QDate&) ) );
    connect( d->popup, SIGNAL( dateClicked(const QDate&) ),
	    this, SIGNAL( valueChanged(const QDate&) ) );
    TimeString::connectChange(this,SLOT(clockChanged()));
    // This is the size policy for the button, not the popup
    setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed) );

    {
	Config config( "qpe" );
	config.setGroup("Time");
	bool onMonday = config.readBoolEntry( "MONDAY" );
	setWeekStartsMonday( onMonday );
    }
    connect( qApp, SIGNAL(weekChanged(bool)), this, SLOT(setWeekStartsMonday(bool)) );
}

/*!
  Returns TRUE if it is possible to select a null date.
*/
bool QPEDateEdit::allowNullDate() const
{
    return mAllowNullButton;
}

/*!
  Display the calendar with weeks starting on Monday if \a startMonday is
  TRUE, otherwise weeks start with Sunday.
*/
void QPEDateEdit::setWeekStartsMonday( bool startMonday )
{
    weekStartsMonday = startMonday;
    d->popup->setWeekStartsMonday( weekStartsMonday );
}

void QPEDateEdit::setNull()
{
    d->popup->close();
    setDate( QDate() );
    emit valueChanged( QDate() );
}

void QPEDateEdit::updateButtonText()
{
    if ( currDate.isValid() ) {
#ifdef QTOPIA_PHONE
	setText( TimeString::shortDate(currDate,df) );
#else
	setText( longFmt ? TimeString::longDateString(currDate,df) : TimeString::dateString(currDate,df) );
#endif
	d->popup->setDate( currDate );
    }
    else {
	setText( tr("None", "no date selected") );
    }
}

/*!
  Return the currently selected date.
*/
QDate QPEDateEdit::date() const
{
    return currDate;
}

/*!
  Set the current date to \a d.
*/
void QPEDateEdit::setDate( const QDate &d )
{
    if (isDown())
	setDown(FALSE);
    if ( d != currDate ) {
	currDate = d;
	updateButtonText();
    }
}

/*!
  Set the long date format to \a format.

  \sa setLongFormat()
*/
void QPEDateEdit::setDateFormat( DateFormat format )
{
    df = format;
    updateButtonText();
}

/*=====================================*/

/*!
  \class QPEDateTimeEdit datetimeedit.h
  \brief The QPEDateTimeEdit class provides a compact widget for selecting
  a date and time.

  QPEDateTimeEdit displays a QPEDateEdit and QPETimeEdit above each other.

  \ingroup qtopiaemb
*/

/*!
  \fn QPEDateTimeEdit::valueChanged( const QDateTime &dt );

  This signal is emitted when the date or time is changed to \a dt.
*/

/*!
  Constructs a QPEDateTimeEdit with the date set to today and the time
  set to 00:00.

    The \a parent and \a name parameters are the standard Qt parent parameters.
*/

QPEDateTimeEdit::QPEDateTimeEdit( QWidget *parent, const char *name)
    : QHBox(parent, name)
{
    de = new QPEDateEdit(this, "date"); // No tr
    te = new QPETimeEdit(this, "time"); // No tr
    connect(de, SIGNAL(valueChanged(const QDate&)),
	    this, SLOT(setDate(const QDate&)));
    connect(te, SIGNAL(valueChanged(const QTime&)),
	    this, SLOT(setTime(const QTime&)));
}

/*!
  Constructs QPEDateTimeEdit with the date and time \a dt.

    The \a parent and \a name parameters are the standard Qt parent parameters.
*/
QPEDateTimeEdit::QPEDateTimeEdit( const QDateTime &dt, QWidget *parent,
	const char *name )
    : QHBox(parent, name)
{
    de = new QPEDateEdit(dt.date(), this, "date"); // No tr
    te = new QPETimeEdit(dt.time(), this, "time"); // No tr
    connect(de, SIGNAL(valueChanged(const QDate&)),
	    this, SLOT(setDate(const QDate&)));
    connect(te, SIGNAL(valueChanged(const QTime&)),
	    this, SLOT(setTime(const QTime&)));
}

/*!
  Destructs QPEDateTimeEdit.
*/
QPEDateTimeEdit::~QPEDateTimeEdit() {}

/*!
  Sets the current date to \a dt.

  \sa setDateTime()
*/
void QPEDateTimeEdit::setDate( const QDate &dt )
{
    de->setDate(dt);
    emit valueChanged(dateTime());
}

/*!
  Sets the current time to \a dt.

  \sa setDateTime()
*/
void QPEDateTimeEdit::setTime( const QTime &dt )
{
    te->setTime(dt);
    emit valueChanged(dateTime());
}

/*!
  Sets the current date and time to \a dt.

  \sa setDate(), setTime()
*/
void QPEDateTimeEdit::setDateTime( const QDateTime &dt )
{
    de->setDate(dt.date());
    te->setTime(dt.time());
    emit valueChanged(dateTime());
}

/*!
  Returns the currently selected date.
*/
QDate QPEDateTimeEdit::date() const
{
    return de->date();
}

/*!
  Returns the currently selected time.
*/
QTime QPEDateTimeEdit::time() const
{
    return te->time();
}

/*!
  Returns the currently selected date and time.
*/
QDateTime QPEDateTimeEdit::dateTime() const
{
    return QDateTime(de->date(), te->time());
}

/*!
  Enables changing the date if \a b is TRUE, otherwise disables
  input.

  \sa dateIsEnabled(), setTimeEnabled()
*/
void QPEDateTimeEdit::setDateEnabled(bool b)
{
    de->setEnabled(b);
}

/*!
  Enables changing the time if \a b is TRUE, otherwise disables
  input.

  \sa timeIsEnabled(), setDateEnabled()
*/
void QPEDateTimeEdit::setTimeEnabled(bool b)
{
    te->setEnabled(b);
}

/*!
  Returns TRUE if changing the date is enabled, otherwise FALSE.

  \sa setDateEnabled()
*/
bool QPEDateTimeEdit::dateIsEnabled() const
{
    return de->isEnabled();
}


/*!
  Returns TRUE if changing the time is enabled, otherwise FALSE.

  \sa setTimeEnabled()
*/
bool QPEDateTimeEdit::timeIsEnabled() const
{
    return te->isEnabled();
}

#include "datetimeedit.moc"
