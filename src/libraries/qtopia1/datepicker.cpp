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

#include <qtopia/config.h>
#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/timestring.h>

#include <qtoolbutton.h>
#include <qdatetime.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qstyle.h>

#include "datepicker.h"
#include <qtopia/calendar.h>

class DatePickerHeader : public QHBox
{
    Q_OBJECT

public:
    DatePickerHeader( QWidget *parent = 0, const char *name = 0 );
    ~DatePickerHeader();
    void setDate( int year, int month );

signals:
    void dateChanged( int year, int month );

protected slots:
    //void keyPressEvent(QKeyEvent *e ) {
	//e->ignore();
    //}

private slots:
    void updateDate();
    void firstMonth();
    void lastMonth();
    void monthBack();
    void monthForward();

private:
    QToolButton *begin, *back, *next, *end;
    QComboBox *month;
    QSpinBox *year;
    int focus;
};

class DatePickerTable : public QTable
{
    Q_OBJECT

public:
    DatePickerTable( QWidget *parent = 0, const char *name = 0);
    ~DatePickerTable();

    QSize minimumSizeHint() const { return sizeHint(); }
    QSize minimumSize() const { return sizeHint(); }

    void setWeekStartsMonday( bool monday );
    bool weekStartsMonday() const { return onMonday; }

    void paintCell(QPainter * p, int row, int col,
	    const QRect & cr, bool selected);

    // 3.0 sig.
    //void paintCell(QPainter * p, int row, int col,
	    //const QRect & cr, bool selected, const QColorGroup & cg);

    void updateContents()
    {
	QScrollView::updateContents(0,0,contentsWidth(), contentsHeight());
    }
signals:
    void clickedPos(int, int);

public slots:
    void timeStringChanged();

protected:
    QSize sizeHint() const;
    void contentsMousePressEvent(QMouseEvent *e)
    {
	pressedCol = columnAt(e->x());
	pressedRow = rowAt(e->y());
    }

    void contentsMouseReleaseEvent(QMouseEvent *e)
    {
	int c = columnAt(e->x());
	int r = rowAt(e->y());

	if (c == pressedCol && r == pressedRow)
	    emit clickedPos(r, c);

	pressedCol = pressedRow = -1;
    }

    QWidget *createEditor(int,int,bool) const { return 0; }


private:
    void setupLabels();

    bool onMonday;
    int pressedRow, pressedCol;
};

void DatePickerTable::timeStringChanged()
{
    setupLabels();
}

DatePickerHeader::DatePickerHeader( QWidget *parent, const char *name )
    : QHBox( parent, name )
{
    setBackgroundMode( PaletteButton );

    begin = new QToolButton( this );
    begin->setIconSet( Resource::loadIconSet( "start" ) );
    begin->setAutoRaise( TRUE );
    begin->setFixedSize( begin->sizeHint() );
    QWhatsThis::add( begin, tr("Show January in the selected year") );

    back = new QToolButton( this );
    back->setIconSet( Resource::loadIconSet( "back" ) );
    back->setAutoRaise( TRUE );
    back->setFixedSize( back->sizeHint() );
    QWhatsThis::add( back, tr("Show the previous month") );

    month = new QComboBox( FALSE, this );
    for ( int i = 0; i < 12; ++i )
	month->insertItem( Calendar::nameOfMonth( i + 1 ) );

    // this is almost certainly wrong on the other end.  
    // date is a uint + 1752 some day.  Thats a lot of days, 11 M years or so.
    year = new QSpinBox( 1753, 3000, 1, this );

    next = new QToolButton( this );
    next->setIconSet( Resource::loadIconSet( "forward" ) );
    next->setAutoRaise( TRUE );
    next->setFixedSize( next->sizeHint() );
    QWhatsThis::add( next, tr("Show the next month") );

    end = new QToolButton( this );
    end->setIconSet( Resource::loadIconSet( "finish" ) );
    end->setAutoRaise( TRUE );
    end->setFixedSize( end->sizeHint() );
    QWhatsThis::add( end, tr("Show December in the selected year") );

    connect( month, SIGNAL( activated( int ) ),
	     this, SLOT( updateDate() ) );
    connect( year, SIGNAL( valueChanged( int ) ),
	     this, SLOT( updateDate() ) );
    connect( begin, SIGNAL( clicked() ),
	     this, SLOT( firstMonth() ) );
    connect( end, SIGNAL( clicked() ),
	     this, SLOT( lastMonth() ) );
    connect( back, SIGNAL( clicked() ),
	     this, SLOT( monthBack() ) );
    connect( next, SIGNAL( clicked() ),
	     this, SLOT( monthForward() ) );
    back->setAutoRepeat( TRUE );
    next->setAutoRepeat( TRUE );
}


DatePickerHeader::~DatePickerHeader()
{

}

void DatePickerHeader::updateDate()
{
    emit dateChanged( year->value(), month->currentItem() + 1 );
}

void DatePickerHeader::firstMonth()
{
    emit dateChanged( year->value(), 1 );
}

void DatePickerHeader::lastMonth()
{
    emit dateChanged( year->value(), 12 );
}


// don't set values.. this will be done on the return hit from monthview.
void DatePickerHeader::monthBack()
{
    if ( month->currentItem() > 0 ) {
	emit dateChanged( year->value(), month->currentItem() );
    } else {
	emit dateChanged( year->value() - 1, 12 );
    }
}

void DatePickerHeader::monthForward()
{
    if ( month->currentItem() < 11 ) {
	emit dateChanged( year->value(), month->currentItem() + 2 );
    } else {
	emit dateChanged( year->value() + 1, 1);
    }
}

void DatePickerHeader::setDate( int y, int m )
{
    blockSignals(TRUE);
    year->setValue( y );
    month->setCurrentItem( m - 1 );
    blockSignals(FALSE);
}

//---------------------------------------------------------------------------
DatePickerTable::DatePickerTable( QWidget *parent, const char *name)
    : QTable( 6, 7, parent, name )
{
    pressedRow = pressedCol = -1;

    Config cfg( "qpe" );
    cfg.setGroup( "Time" );
    onMonday = cfg.readBoolEntry( "MONDAY" );

    horizontalHeader()->setResizeEnabled( FALSE );
    // we have to do this here... or suffer the consequences later...
    int i;
    for ( i = 0; i < 7; i++ ){
	horizontalHeader()->resizeSection( i, 30 );
	setColumnStretchable( i, TRUE );
    }
    setupLabels();

    verticalHeader()->hide();
    setLeftMargin( 0 );
    for ( i = 0; i < 6; ++i )
	    setRowStretchable( i, TRUE );

    setSelectionMode( NoSelection );

    setVScrollBarMode( AlwaysOff );
    setHScrollBarMode( AlwaysOff );
}

DatePickerTable::~DatePickerTable()
{
}


QSize DatePickerTable::sizeHint() const
{
    return QSize( 7*30+4, 6*25 );
}

void DatePickerTable::setWeekStartsMonday( bool monday )
{
    onMonday = monday;
    setupLabels();
}

void DatePickerTable::paintCell(QPainter * p, int row, int col,
	const QRect & cr, bool selected)
{
    //QDate cDay = Calendar::dayOnCalendar( year, month, onMonday, row, col);

#if defined(Q_WS_WIN)
    const QColorGroup &cg = ( style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus ) ? palette().inactive() : colorGroup() );
#else
    const QColorGroup &cg = colorGroup();
#endif

    ((QPEDatePicker *)parentWidget())->paintCell(row, col, p, cr, selected, cg);

}

void DatePickerTable::setupLabels()
{
    for ( int i = 0; i < 7; ++i ) {
// 	horizontalHeader()->resizeSection( i, 30 );
// 	setColumnStretchable( i, TRUE );
	if ( onMonday )
	    horizontalHeader()->setLabel( i, Calendar::nameOfDay( i + 1 ) );
	else {
	    if ( i == 0 )
		horizontalHeader()->setLabel( i, Calendar::nameOfDay( 7 ) );
	    else
		horizontalHeader()->setLabel( i, Calendar::nameOfDay( i ) );
	}
    }
}


//---------------------------------------------------------------------------

/*!
  \class QPEDatePicker datepicker.h
  \brief The QPEDatePicker class allows a date to be selected from a
  calendar view.

  QPEDatePicker comprises a header to select month and year and a
  calendar view to select the date.

  \ingroup qtopiaemb
*/

/*!
  \fn void QPEDatePicker::dateClicked( const QDate &date );

  This signal is emitted when a date in the calendar is clicked.
  \a date contains the date that was clicked 
*/

/*!
  Constructs a QPEDatePicker.
*/
QPEDatePicker::QPEDatePicker( QWidget *parent, const char *name)
    : QVBox( parent, name )
{
    year = 1970;		// Default to epoch.
    month = 1;

    header = new DatePickerHeader( this, "DatePickerHeader" );
    table = new DatePickerTable( this, "DatePickerTable" );
    table->setFrameStyle( QFrame::NoFrame );
    header->setDate( year, month );

    table->updateContents();
    connect( header, SIGNAL( dateChanged( int, int ) ),
	     this, SLOT( setDate( int, int ) ) );
    connect( table, SIGNAL( clickedPos( int, int ) ),
	     this, SLOT( calendarClicked(int, int) ) );
    connect( table, SIGNAL( currentChanged( int, int ) ),
	     this, SLOT( calendarChanged(int, int) ) );
    connect( qApp, SIGNAL(weekChanged(bool)), this,
	     SLOT(setWeekStartsMonday(bool)) );
    TimeString::connectChange(table, SLOT(timeStringChanged()));
    setDate(QDate::currentDate());
    setFocusPolicy(StrongFocus);
    setFocus();
}

/*!
  Destructs QPEDatePicker.
*/
QPEDatePicker::~QPEDatePicker()
{

}

void QPEDatePicker::paintCell(int row, int col, QPainter * p,
	const QRect & cr, bool, const QColorGroup & cg)
{
    QDate cDay = Calendar::dateAtCoord( year, month, row, col,
	    table->weekStartsMonday());

    paintDay(cDay, p, cr, cg);
}

/*!
  Paints a single day \a cDay in the calendar using \a p.  The cell
  geometry is \a cr.

  The default implementation draws the day of the month in the top left
  corner of the cell.
*/
void QPEDatePicker::paintDay(const QDate &cDay, QPainter *p, const QRect &cr,
	const QColorGroup &cg)
{
    p->save();
    QColorGroup mygroup = cg;
    if (cDay.month() == month)
	mygroup.setBrush( QColorGroup::Base, white );
    else
	mygroup.setBrush( QColorGroup::Base, QColor( 224, 224, 224 ) );

    mygroup.setColor( QColorGroup::Text, black );

    paintDayBackground(cDay, p, cr, mygroup);

    QPen tPen = p->pen();
    p->setPen( cg.mid() );
    p->drawLine( 0, cr.height() - 1, cr.width() - 1, cr.height() - 1 );
    p->drawLine( cr.width() - 1, 0, cr.width() - 1, cr.height() - 1 );
    p->setPen(tPen);

    // base font size of geometry of first cell;
    QSize cSize = table->cellGeometry(0,0).size();
    p->setPen( mygroup.text() );

    QFont f = p->font();
    QFontMetrics fmtest( f );
    if (fmtest.height() > QMAX(10, cSize.height() / 3)) {
	f.setPointSize( QMAX(( f.pointSize() / 3 ) * 2, 8 ) );
	p->setFont( f );
    }
    QFontMetrics fm( f );
    if (QDate::currentDate() == cDay) {
	p->setPen(QColor(255,255,255));
	p->fillRect(1, 1, fm.width(QString::number( cDay.day() )) + 1,
		fm.height(), QColor(0,0,0));
    }
    p->drawText( 1, 1 + fm.ascent(), QString::number( cDay.day() ) );
    p->restore();
}

/*!
  Paints the background of a single day in the calendar using \a p.
  The cell geometry is \a cr.

  The default implementation fills with the base color.
*/
void QPEDatePicker::paintDayBackground(const QDate &, QPainter *p,
	const QRect &cr, const QColorGroup &cg)
{
    p->fillRect( 0, 0, cr.width(), cr.height(),
	    cg.brush( QColorGroup::Base ) );
}

/*!
  Returns TRUE if the beginning of the week is Monday.

  \sa setWeekStartsMonday()
*/
bool QPEDatePicker::weekStartsMonday() const
{
    return table->weekStartsMonday();
}

/*! \internal
*/
void QPEDatePicker::updateContents()
{
    table->updateContents();
}

/*!
  Sets the selected date to year \a y and month \a m.  The current day of
  the month is retained unless it falls outside the number of days in the
  selected date.
*/
void QPEDatePicker::setDate( int y, int m )
{
    /* only change the date if this is a different date,
     * other wise we may mistakenly overide the day */
    if ( y != year || m != month ) {
	QDate nd( y, m, 1 );
	if ( nd.daysInMonth() < day )
	    setDate(y, m, nd.daysInMonth());
	else
	    setDate(y, m, day);
    }
}

/*!
  Sets the current date to year \a y, month \a m and day \a d.
*/
void QPEDatePicker::setDate( int y, int m, int d )
{
    setDate(QDate(y, m, d));
}

/* called when we wish to close or pass back the date */
void QPEDatePicker::calendarClicked(int r, int c)
{
    calendarChanged(r, c);

    emit dateClicked(QDate(year, month, day));
}

void QPEDatePicker::calendarChanged(int r, int c)
{
    setDate( Calendar::dateAtCoord(year, month, r, c, table->weekStartsMonday()) );
}

/*!
  Sets the selected date to \a d.
*/
void QPEDatePicker::setDate( const QDate &d )
{
    if (year != d.year() || month != d.month()) {
	year = d.year();
	month = d.month();
	header->setDate( d.year(), d.month());
	table->updateContents();
    } else if (day == d.day())
	return;

    day = d.day();
    int r, c;
    Calendar::coordForDate(d.year(), d.month(), d, r, c, table->weekStartsMonday());
    table->setCurrentCell(r,c);
}

/*!
  Returns the selected date.
*/
QDate  QPEDatePicker::selectedDate() const
{
    if ( !table )
	return QDate::currentDate();
    return QDate( year, month, day );
}

/*!
  Display the calendar with weeks starting on Monday if \a startMonday is
  TRUE, otherwise weeks start with Sunday.

  \sa weekStartsMonday()
*/
void QPEDatePicker::setWeekStartsMonday( bool startMonday )
{
    table->setWeekStartsMonday( startMonday );
}

/*!
  \internal
*/
void QPEDatePicker::keyPressEvent( QKeyEvent *e )
{
    switch(e->key()) {
	case Key_Up:
	    setDate(QDate(year, month, day).addDays(-7));
	    break;
	case Key_Down:
	    setDate(QDate(year, month, day).addDays(7));
	    break;
	case Key_Left:
	    setDate(QDate(year, month, day).addDays(-1));
	    break;
	case Key_Right:
	    setDate(QDate(year, month, day).addDays(1));
	    break;
	case Key_Space:
	    qWarning("space");
	    emit dateClicked(QDate(year, month, day));
	    break;
	default:
	    qWarning("ignore");
	    e->ignore();
	    break;
    }
}
#include "datepicker.moc"
