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
#include <qspinbox.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qwhatsthis.h>

#include "datepicker.h"
#include <qtopia/pim/calendar.h>

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

    void setWeekStartsOnMonday( bool monday );
    bool weekStartsOnMonday() const { return onMonday; }

    void paintCell(QPainter * p, int row, int col, 
	    const QRect & cr, bool selected);

    // 3.0 sig.
    //void paintCell(QPainter * p, int row, int col, 
	    //const QRect & cr, bool selected, const QColorGroup & cg);

    void repaintContents()
    {
	//QScrollView::repaintContents(0,0,contentsWidth(), contentsHeight(), FALSE);
	QScrollView::updateContents(0,0,contentsWidth(), contentsHeight());
    }
signals:
    void clickedPos(int, int);

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

protected slots:

    //void keyPressEvent(QKeyEvent *e ) {
//	e->ignore();
 //   }

private:
    void setupLabels();

    bool onMonday;
    int pressedRow, pressedCol;
};


DatePickerHeader::DatePickerHeader( QWidget *parent, const char *name )
    : QHBox( parent, name )
{
    setBackgroundMode( PaletteButton );

    begin = new QToolButton( this );
    begin->setPixmap( Resource::loadPixmap( "start" ) );
    begin->setAutoRaise( TRUE );
    begin->setFixedSize( begin->sizeHint() );
    QWhatsThis::add( begin, tr("Show January in the selected year") );

    back = new QToolButton( this );
    back->setPixmap( Resource::loadPixmap( "back" ) );
    back->setAutoRaise( TRUE );
    back->setFixedSize( back->sizeHint() );
    QWhatsThis::add( back, tr("Show the previous month") );

    month = new QComboBox( FALSE, this );
    for ( int i = 0; i < 12; ++i )
	month->insertItem( PimCalendar::nameOfMonth( i + 1 ) );

    year = new QSpinBox( 1970, 2037, 1, this );

    next = new QToolButton( this );
    next->setPixmap( Resource::loadPixmap( "forward" ) );
    next->setAutoRaise( TRUE );
    next->setFixedSize( next->sizeHint() );
    QWhatsThis::add( next, tr("Show the next month") );

    end = new QToolButton( this );
    end->setPixmap( Resource::loadPixmap( "finish" ) );
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

void DatePickerTable::setWeekStartsOnMonday( bool monday )
{
    onMonday = monday;
    setupLabels();
}

void DatePickerTable::paintCell(QPainter * p, int row, int col, 
	const QRect & cr, bool selected)
{    
    //QDate cDay = PimCalendar::dayOnCalendar( year, month, onMonday, row, col);

#if defined(Q_WS_WIN)
    const QColorGroup &cg = ( !drawActiveSelection && style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus ) ? palette().inactive() : colorGroup() );
#else
    const QColorGroup &cg = colorGroup();
#endif

    ((DatePicker *)parentWidget())->paintCell(row, col, p, cr, selected, cg);
    //((DatePicker *)parentWidget())->paintDay(cDay, p, cr, selected, cg);

}

void DatePickerTable::setupLabels()
{
    for ( int i = 0; i < 7; ++i ) {
// 	horizontalHeader()->resizeSection( i, 30 );
// 	setColumnStretchable( i, TRUE );
	if ( onMonday )
	    horizontalHeader()->setLabel( i, PimCalendar::nameOfDay( i + 1 ) );
	else {
	    if ( i == 0 )
		horizontalHeader()->setLabel( i, PimCalendar::nameOfDay( 7 ) );
	    else
		horizontalHeader()->setLabel( i, PimCalendar::nameOfDay( i ) );
	}
    }
}


//---------------------------------------------------------------------------

DatePicker::DatePicker( QWidget *parent, const char *name, bool ac )
    : QVBox( parent, name ),
      autoClose( ac )
{
    year = 1970;		// Default to epoch.
    month = 1;

    header = new DatePickerHeader( this, "DatePickerHeader" );
    table = new DatePickerTable( this, "DatePickerTable" );
    table->setFrameStyle( QFrame::NoFrame );
    header->setDate( year, month );
    table->repaintContents();
    connect( header, SIGNAL( dateChanged( int, int ) ),
	     this, SLOT( setDate( int, int ) ) );
    connect( table, SIGNAL( clickedPos( int, int ) ),
	     this, SLOT( calendarClicked(int, int) ) );
    connect( table, SIGNAL( currentChanged( int, int ) ),
	     this, SLOT( calendarChanged(int, int) ) );
    connect( qApp, SIGNAL(weekChanged(bool)), this,
	     SLOT(slotWeekChange(bool)) );
    setDate(QDate::currentDate());
    setFocusPolicy(StrongFocus);
    setFocus();
}

DatePicker::~DatePicker()
{

}

void DatePicker::paintCell(int row, int col, QPainter * p,
	const QRect & cr, bool selected, const QColorGroup & cg)
{    
    QDate cDay = PimCalendar::dateAtPosition( year, month, 
	    table->weekStartsOnMonday(), row, col);

    paintDay(cDay, p, cr, selected, cg);
}

void DatePicker::paintDay(const QDate &cDay, QPainter *p, const QRect &cr, 
	bool selected, const QColorGroup &cg)
{
    p->save();
    QColorGroup mygroup = cg;
    if (cDay.month() == month)
	mygroup.setBrush( QColorGroup::Base, white );
    else
	mygroup.setBrush( QColorGroup::Base, QColor( 224, 224, 224 ) );

    mygroup.setColor( QColorGroup::Text, black );

    p->fillRect( 0, 0, cr.width(), cr.height(), selected ? mygroup.brush( QColorGroup::Highlight ) : mygroup.brush( QColorGroup::Base ) );

    QPen tPen = p->pen();
    p->setPen( cg.mid() );
    p->drawLine( 0, cr.height() - 1, cr.width() - 1, cr.height() - 1 );
    p->drawLine( cr.width() - 1, 0, cr.width() - 1, cr.height() - 1 );
    p->setPen(tPen);

    if ( selected )
	p->setPen( mygroup.highlightedText() );
    else
	p->setPen( mygroup.text() );

    QFont f = p->font();
    f.setPointSize( ( f.pointSize() / 3 ) * 2 );
    p->setFont( f );
    QFontMetrics fm( f );
    if (QDate::currentDate() == cDay) {
	p->setPen(QColor(255,255,255));
	p->fillRect(1, 1, fm.width(QString::number( cDay.day() )) + 1,
		fm.height(), QColor(0,0,0));
    }
    p->drawText( 1, 1 + fm.ascent(), QString::number( cDay.day() ) );
    p->restore();
}

bool DatePicker::weekStartsOnMonday() const
{
    return table->weekStartsOnMonday();
}

void DatePicker::repaintContents()
{
    table->repaintContents();
}

void DatePicker::setDate( int y, int m )
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

void DatePicker::setDate( int y, int m, int d )
{
    if (year != y || month != m) {
	year = y;
	month = m;
	header->setDate( y, m);
	table->repaintContents();
    } else if (day == d)
	return;

    day = d;
    int r, c;
    PimCalendar::positionOfDate(y, m, table->weekStartsOnMonday(), QDate(y,m,d), r, c);
    table->setCurrentCell(r,c);
}

/* called when we wish to close or pass back the date */
void DatePicker::calendarClicked(int r, int c)
{
    calendarChanged(r, c);

    emit dateClicked(year, month, day);

    if ( autoClose && parentWidget() )
	parentWidget()->close();
}

void DatePicker::calendarChanged(int r, int c)
{
    setDate( PimCalendar::dateAtPosition(year, month, table->weekStartsOnMonday(), r, c) );
}

void DatePicker::setDate( QDate d)
{
    setDate(d.year(), d.month(), d.day());
}

QDate  DatePicker::selectedDate() const
{
    if ( !table )
	return QDate::currentDate();
    return QDate( year, month, day );
}

void DatePicker::slotWeekChange( bool startOnMonday )
{
    table->setWeekStartsOnMonday( startOnMonday );
}

void DatePicker::keyPressEvent( QKeyEvent *e )
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
	    emit dateClicked(year, month, day);
	    if ( autoClose && parentWidget() )
		parentWidget()->close();
	    break;
	default:
	    qWarning("ignore");
	    e->ignore();
	    break;
    }
}

//---------------------------------------------------------------------------

QPEDateButton::QPEDateButton( QWidget *parent, const char * name )
    :QPushButton( parent, name )
{
    longFmt = FALSE;
    init();
}

QPEDateButton::QPEDateButton( bool longDate, QWidget *parent, const char * name )
    :QPushButton( parent, name )
{
    init();
    setLongFormat(longDate);
}

void QPEDateButton::setLongFormat( bool l )
{
    longFmt = l;
    setText( longFmt ? TimeString::longDateString( currDate, df ) :
	    TimeString::shortDate( currDate, df ) ); 
}

void QPEDateButton::init()
{
    //df = ::DateFormat('/', ::DateFormat::MonthDayYear, ::DateFormat::MonthDayYear);
    df = TimeString::currentDateFormat();
    QPopupMenu *popup = new QPopupMenu( this );
    monthView = new DatePicker( popup, 0, TRUE );

    //
    // Geometry information for the DatePicker widget is
    // setup by the QPopupMenu (parent).  However at 176x220,
    // the widget hangs off the side of the screen.  Force the
    // widget to be bounded by the desktop width.
    //
    monthView->setMaximumWidth(qApp->desktop()->width());

    popup->insertItem( monthView );
    connect( monthView, SIGNAL( dateClicked( int, int, int ) ),
	    this, SLOT( setDate( int, int, int ) ) );
    connect( monthView, SIGNAL( dateClicked( int, int, int ) ),
	    this, SIGNAL( dateSelected( int, int, int ) ) );
    connect( qApp, SIGNAL(dateFormatChanged(DateFormat)), this, SLOT(setDateFormat(DateFormat)));
    setPopup( popup );
    popup->setFocusPolicy(NoFocus);
    setDate( QDate::currentDate() );
    setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed) );
}

void QPEDateButton::setWeekStartsMonday( int b )
{
    weekStartsMonday = b;
    monthView->slotWeekChange( weekStartsMonday );
}

void QPEDateButton::setDate( int y, int m, int d )
{
    setDate( QDate( y,m,d) );
}

void QPEDateButton::setDate( QDate d )
{
    if ( d != currDate ) {
	currDate = d;
	setText( longFmt ? TimeString::longDateString( d, df ) :
		 TimeString::shortDate( d, df ) ); 
	monthView->setDate( currDate.year(), currDate.month(), currDate.day() );
    }
}

void QPEDateButton::setDateFormat( DateFormat f )
{
    df = f;
    setText( longFmt ? TimeString::longDateString( currDate, df ) :
	     TimeString::shortDate( currDate, df ) ); 
}

#include "datepicker.moc"
