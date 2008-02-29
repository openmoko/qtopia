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

#define QTOPIA_INTERNAL_FD

#include "datebook.h"
#include "dayview.h"
#include "datebooksettings.h"
#include "datebookweek.h"
#include "dateentryimpl.h"
#include "monthview.h"
#include "finddialog.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/qpedebug.h>
#include <qtopia/pim/event.h>
#ifdef Q_WS_QWS
#include <qtopia/ir.h>
#endif
#include <qtopia/qpemenubar.h>
#include <qtopia/qpemessagebox.h>
#include <qtopia/resource.h>
#include <qtopia/sound.h>
#include <qtopia/timestring.h>
#include <qtopia/qpetoolbar.h>
#include <qtopia/tzselect.h>
#include <qtopia/xmlreader.h>
#include <qtopia/applnk.h>
#include <qtopia/pim/private/eventio_p.h>

#include <qaction.h>
#include <qdatetime.h>
#include <qdialog.h>
#include <qdir.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qtl.h>
#include <qwidgetstack.h>
#include <qvbox.h>
#include <qregexp.h>
//#ifdef QWS
////#include <qwindowsystem_qws.h>
////#include <qcopchannel_qws.h>
////#endif
#include <qarray.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifndef Q_OS_WIN32
#include <unistd.h>
#endif

#include <stdlib.h>

#define DAY 1
#define WEEK 2
#define MONTH 3


// Make QScrollView in AutoOneFit mode use the minimum horizontal size
// instead of the sizeHint() so that the widgets fit horizontally.
class VScrollBox : public QVBox
{
public:
    VScrollBox( QWidget *parent, const char *name=0 )
	: QVBox( parent, name ) {}
#if defined(_WS_QWS_)
    QSize sizeHint() const {
	int width = QVBox::sizeHint().width();
	if ( width > qApp->desktop()->width()-style().scrollBarExtent().width() )
	    width = qApp->desktop()->width()-style().scrollBarExtent().width();
	width = QMAX(width, QVBox::minimumSize().width());
	return QSize( width, QVBox::sizeHint().height());
    }
#endif
};


DateBook::DateBook( QWidget *parent, const char *, WFlags f )
    : QMainWindow( parent, "datebook", f ),
      aPreset( FALSE ),
      presetTime( -1 ),
      startTime( 8 ), // an acceptable default
      syncing(FALSE),
      inSearch(FALSE),
      exceptionMb(0)
{
    int timingLoad, timingOther;

    QTime t;
    t.start();
    db = new DateBookTable(this);
    timingLoad = t.elapsed();
    loadSettings();
    setCaption( tr("Calendar") );
    setIcon( Resource::loadPixmap( "DateBook" ) );

    bool thinScreen = QApplication::desktop()->width() < 200;

    setToolBarsMovable( FALSE );
    setBackgroundMode( PaletteButton );

    QPEToolBar *bar = new QPEToolBar( this );
    bar->setHorizontalStretchable( TRUE );

    QPEMenuBar *mb = new QPEMenuBar( bar );
    mb->setMargin( 0 );

    QPEToolBar *sub_bar = new QPEToolBar(this);

    QPopupMenu *eventMenu = new QPopupMenu( this );
    QPopupMenu *view = new QPopupMenu( this );

    mb->insertItem( tr( "Event" ), eventMenu );
    mb->insertItem( tr( "View" ), view );

    QActionGroup *g = new QActionGroup( this );
    g->setExclusive( TRUE );

    QAction *a = new QAction( tr( "New Event" ), Resource::loadIconSet( "new" ),
                              QString::null, 0, this, 0 );
    a->setWhatsThis( tr("Create a new event") );
    connect( a, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    a->addTo( sub_bar );
    a->addTo( eventMenu );

    editAction = new QAction( tr( "Edit Event" ),
	    Resource::loadIconSet( "edit" ), QString::null, 0, this, 0 );
    connect( editAction, SIGNAL( activated() ), this, SLOT( editCurrentEvent() ) );
    editAction->setWhatsThis( tr("Edit the selected event") );
    editAction->addTo( eventMenu );

    removeAction = new QAction( tr( "Delete Event" ),
	    Resource::loadIconSet( "trash" ), QString::null, 0, this, 0 );
    connect( removeAction, SIGNAL( activated() ), this, SLOT( removeCurrentEvent() ) );
    removeAction->setWhatsThis( tr("Delete the selected event") );
    removeAction->addTo( eventMenu );

#ifdef Q_WS_QWS
    if (Ir::supported()) {
	beamAction = new QAction( tr( "Beam Event" ),
	    Resource::loadIconSet( "beam" ), QString::null, 0, this, 0 );
	connect( beamAction, SIGNAL( activated() ),
	    this, SLOT( beamCurrentEvent() ) );
	beamAction->setWhatsThis( tr("Beam the selected event") );
	beamAction->addTo( eventMenu );
    }
#endif

    eventMenu->insertSeparator();

    /*
    a = new QAction( tr( "Delete Events" ), Resource::loadIconSet( "trash" ),
                            QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    a->addTo( sub_bar );
    a->addTo( eventMenu );
    */

    a = new QAction( tr( "Day" ), Resource::loadIconSet( "day" ), QString::null, 0, g, 0 );
    a->setWhatsThis( tr("Show selected day's events") );
    if (!thinScreen)
	a->addTo( sub_bar );
    a->addTo( view );
    a->setToggleAction( TRUE );
    a->setOn( TRUE );
    dayAction = a;
    connect( dayAction, SIGNAL( activated() ), this, SLOT( viewDay() ) );
    a = new QAction( tr( "Week" ), Resource::loadIconSet( "week" ), QString::null, 0, g, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( viewWeek() ) );
    a->setWhatsThis( tr("Show selected week's events") );
    if (!thinScreen)
	a->addTo( sub_bar );
    a->addTo( view );
    a->setToggleAction( TRUE );
    weekAction = a;
    a = new QAction( tr( "Month" ), Resource::loadIconSet( "month" ), QString::null, 0, g, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( viewMonth() ) );
    a->setWhatsThis( tr("Show selected months's events") );
    if (!thinScreen)
	a->addTo( sub_bar );
    a->addTo( view );
    a->setToggleAction( TRUE );
    monthAction = a;

    a = new QAction( tr( "Find Event" ), Resource::loadIconSet( "find" ), QString::null, 0, g, 0 );
    connect( a, SIGNAL(activated()), this, SLOT(slotFind()) );
    //a->addTo( sub_bar );
    a->addTo( eventMenu );

    a = new QAction( tr( "Today" ), Resource::loadIconSet( "today" ), QString::null, 0, g, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( slotToday() ) );
    a->setWhatsThis( tr("Show today's events") );
    a->addTo( sub_bar );
    a->addTo( view );

    view->insertSeparator();
    a = new QAction( tr( "Settings..." ), QString::null, 0, g );
    connect( a, SIGNAL( activated() ), this, SLOT( showSettings() ) );
    a->addTo( view );

    views = new QWidgetStack( this );
    setCentralWidget( views );

    dayView = 0;
    weekView = 0;
    monthView = 0;

    timingOther = t.elapsed();
    viewDay();
    TimeString::connectChange(this, SLOT(changeClock()) );
    connect( qApp, SIGNAL(weekChanged(bool)),
             this, SLOT(changeWeek(bool)) );

    connect( qApp, SIGNAL( appMessage(const QCString &, const QByteArray &) ), 
	    this, SLOT( appMessage(const QCString &, const QByteArray &) ) );

    connect( qApp, SIGNAL( timeChanged() ), 
	    this, SLOT( refreshWidgets() ) );

    connect( qApp, SIGNAL( flush() ), 
	    this, SLOT( flush() ) );

    connect( qApp, SIGNAL( reload() ), 
	    this, SLOT( reload() ) );
}

void DateBook::refreshWidgets()
{
    // update active view!
    if ( dayAction->isOn() )
	dayView->redraw();
	//viewDay();
    else if ( weekAction->isOn() )
	weekView->redraw();
	//viewWeek();
    else if ( monthAction->isOn() )
	monthView->updateOccurrences();
	//viewMonth();
}

DateBook::~DateBook()
{
}

void DateBook::showSettings()
{
    DateBookSettings frmSettings( this );
    frmSettings.setStartTime( startTime );
    frmSettings.setAlarmPreset( aPreset, presetTime );

    if ( QPEApplication::execDialog(&frmSettings) ) {
	aPreset = frmSettings.alarmPreset();
	presetTime = frmSettings.presetTime();
	startTime = frmSettings.startTime();
	if ( dayView )
	    dayView->setDayStarts( startTime );
	if ( weekView )
	    weekView->setDayStarts( startTime );
	saveSettings();

	// make the change obvious
	if ( views->visibleWidget() ) {
	    if ( views->visibleWidget() == dayView )
		dayView->redraw();
	    else if ( views->visibleWidget() == weekView )
		weekView->redraw();
	}
    }
}

void DateBook::fileNew()
{
    newEvent("");
}

QString DateBook::checkEvent(const PimEvent &e)
{
    /* check if overlaps with itself */
    bool checkFailed = FALSE;

    /* check the next 12 repeats. should catch most problems */
    QDate current_date = e.end().date();
    int duration = e.start().date().daysTo(e.end().date());
    PimEvent previous = e;
    for(int i = 0; i < 12; i++)
    {
	bool ok;
	QDateTime next;
	next = e.nextOccurrence(current_date.addDays(1), &ok);
	if (!ok)
	    break;  // no more repeats
	if(next < previous.end()) {
	    checkFailed = TRUE;
	    break;
	}
	current_date = next.date().addDays(duration);
    }


    if(checkFailed)
	return tr("Event duration is potentially longer\n"
		  "than interval between repeats.");

    return QString::null;
}

QDate DateBook::currentDate()
{
    QDate d = QDate::currentDate();

    if ( dayView && views->visibleWidget() == dayView ) {
	d = dayView->currentDate();
    } else if ( weekView && views->visibleWidget() == weekView ) {
        d = weekView->currentDate();
    } else if ( monthView && views->visibleWidget() == monthView ) {
	d = monthView->selectedDate();
    }

    return d;
}

void DateBook::viewToday()
{
    viewDay( QDate::currentDate() );
}

void DateBook::viewDay(const QDate& dt)
{
    initDay();
    dayAction->setOn( TRUE );
    dayView->selectDate( dt );
    views->raiseWidget( dayView );
    dayView->setFocus();
    dayView->redraw();

    updateIcons();
}

void DateBook::viewDay()
{
    viewDay( currentDate() );
}

void DateBook::viewWeek()
{
    viewWeek( currentDate() );
}

void DateBook::viewWeek( const QDate& dt )
{
    initWeek();
    weekAction->setOn( TRUE );
    weekView->selectDate( dt );
    views->raiseWidget( weekView );
    weekView->redraw();

    updateIcons();
}

void DateBook::viewMonth()
{
    viewMonth( currentDate() );
}

void DateBook::viewMonth( const QDate& dt)
{
    initMonth();
    monthAction->setOn( TRUE );
    monthView->setDate( dt );
    views->raiseWidget( monthView );
    monthView->updateOccurrences();

    updateIcons();
}

void DateBook::updateIcons()
{
    bool s = eventSelected();

    removeAction->setEnabled(s);
#ifdef Q_WS_QWS
    if (Ir::supported()) {
	beamAction->setEnabled(s);
    }
#endif
    editAction->setEnabled(s);
}

void DateBook::initExceptionMb() {
    exceptionMb = new QMessageBox(tr("Calendar"),
	    tr("Edit all events in this series or just this single event."),
	    QMessageBox::Warning, QMessageBox::Yes,
	    QMessageBox::No | QMessageBox::Default, QMessageBox::Cancel, this);

    exceptionMb->setButtonText(QMessageBox::Yes, tr("All"));
    exceptionMb->setButtonText(QMessageBox::No, tr("Single","1 event, not all"));
    exceptionMb->setTextFormat(RichText);
}

// a bit to extreme.
// should be possible to check each exception for if it still fits
// on a valid date.
bool affectsExceptions(const PimEvent &ne, const PimEvent &oe)
{
    if (oe.hasExceptions()) {
	if (ne.start() != oe.start())
	    return TRUE;
	if (ne.frequency() != oe.frequency())
	    return TRUE;
	if (ne.repeatType() != oe.repeatType())
	    return TRUE;
	if ( ((PrEvent &)ne).p_weekMask() != ((PrEvent &)oe).p_weekMask())
	    return TRUE;
    }
    return FALSE;
}

void DateBook::editOccurrence( const Occurrence &ev )
{
    if (syncing) {
	QMessageBox::warning( this, tr("Calendar"),
	                      tr( "Can not edit data, currently syncing") );
	return;
    }


    // if this event is an exception, or has exceptions we may need to do somethign different.
    bool asException;
    PimEvent e;
    if (ev.event().isException() || ev.event().hasRepeat()) {
	// ask if just this one or is series?
	if (!exceptionMb)
	    initExceptionMb();
	exceptionMb->setText(tr("Edit all events in this series or just this single event."));
	switch (exceptionMb->exec()) {
	    default:
		return;
	    case QMessageBox::Yes:
		if (ev.event().isException()) {
		    e = db->find(ev.event().seriesUid());
		} else {
		    e = ev.event();
		}
		asException = FALSE;
		break;
	    case QMessageBox::No:
		if (ev.event().isException()) {
		    e = ev.event();
		    asException = FALSE;
		} else {
		    e = ev.event();
		    // modify e to be an exceptional event.
		    // with no uid (yet).
		    e.setSeriesUid(ev.event().uid());
		    e.setRepeatType(PimEvent::NoRepeat);
		    e.setStart(ev.start());
		    e.clearExceptions();
		    asException = TRUE;
		}
		break;
	}
    } else {
	e = ev.event();
	asException = FALSE;
    }

    // workaround added for text input.
    QDialog editDlg( this, 0, TRUE );
    DateEntry *entry;
    if (e.isException())
	editDlg.setCaption( tr("Edit Event Exception") );
    else
	editDlg.setCaption( tr("Edit Event") );
    QVBoxLayout *vb = new QVBoxLayout( &editDlg );
    QScrollView *sv = new QScrollView( &editDlg, "scrollview" );
    sv->setResizePolicy( QScrollView::AutoOneFit );
    // KLUDGE!!!
    sv->setHScrollBarMode( QScrollView::AlwaysOff );
    vb->addWidget( sv );
    VScrollBox *vsb = new VScrollBox( &editDlg );
    entry = new DateEntry( onMonday, e, vsb, "editor" ); // No tr

    // connect the qApp stuff.
    connect( qApp, SIGNAL(weekChanged(bool)),
	     entry, SLOT(setWeekStartsMonday(bool)) );

    //entry->timezone->setEnabled(FALSE);
    sv->addChild( vsb );

    while (QPEApplication::execDialog(&editDlg) ) {
	PimEvent newEv = entry->event();
	QString error = checkEvent(newEv);
	if (!error.isNull()) {
	    if (QMessageBox::warning(this, "error box",
			error, "Fix it", "Continue", 0, 0, 1) == 0)
		continue;
	}
	QUuid u;
	if (asException) {
	    u = db->addException(ev.date(),ev.event(), newEv);
	} else {
	    if (affectsExceptions(newEv, e)) {
		if (QMessageBox::warning(this, tr("Calendar"),
			    tr( "<p>Changes to the start time or recurrence pattern"
				" of this event will cause all exceptions to this event"
				" to be lost.  Continue?"),
			    QMessageBox::Ok, QMessageBox::Cancel|QMessageBox::Default
			    ) != QMessageBox::Ok)
		    return;
		/// need to clear exceptions
		db->removeExceptions(newEv);
		newEv.clearExceptions();
	    }
	    db->updateEvent(newEv);
	    u = newEv.uid();
	}
	emit newEvent();
	if ( views->visibleWidget() == dayView ) {
	    bool ok;
	    PimEvent e = db->find( u, &ok );
	    if ( ok )
		dayView->setCurrentEvent( e );
	}
	break;
    }
}

void DateBook::removeOccurrence( const Occurrence &o )
{
    PimEvent e = o.event();
    if (syncing) {
	QMessageBox::warning( this, tr("Calendar"),
	                      tr( "Can not edit data, currently syncing") );
	return;
    }

    QString strName = e.description();
    if (e.isException() || e.hasRepeat()) {
	// ask if just this one or is series?
	if (!exceptionMb)
	    initExceptionMb();
	exceptionMb->setText(
		tr("Delete all events in this series or just this single event:<br>%1")
		.arg(strName)
		);
	switch (exceptionMb->exec()) {
	    default:
		return;
	    case QMessageBox::Yes:
		if (e.isException()) {
		    e = db->find(e.seriesUid());
		}
		db->removeEvent( e );
		break;
	    case QMessageBox::No:
		if (e.isException()) {
		    db->removeEvent( e );
		} else {
		    db->addException( o.start().date(), e );
		}
	}
    } else  {
	if ( !QPEMessageBox::confirmDelete( this, tr( "Calendar" ),strName ) )
	    return;
	db->removeEvent( e );
    }

    if ( views->visibleWidget() == dayView && dayView )
        dayView->redraw();
}

void DateBook::addEvent( const PimEvent &e )
{
    QDate d = e.start().date();
    initDay();
    dayView->selectDate( d );
}

void DateBook::editCurrentEvent()
{
    if (eventSelected())
	editOccurrence(currentOccurrence());
}

void DateBook::removeCurrentEvent()
{
    if (eventSelected())
	removeOccurrence(currentOccurrence());
}

void DateBook::beamCurrentEvent()
{
#ifdef Q_WS_QWS
    if (eventSelected())
	beamEvent(currentEvent());
#endif
}


bool DateBook::eventSelected() const
{
    if (views->visibleWidget() && views->visibleWidget() == dayView) {
	return dayView->hasSelection();
    }
    return FALSE;
}

PimEvent DateBook::currentEvent() const
{
    return dayView->currentEvent();
}

Occurrence DateBook::currentOccurrence() const
{
    return dayView->currentItem();
}

void DateBook::showDay( int year, int month, int day )
{
    showDay(QDate(year, month, day));
}

void DateBook::showDay( const QDate &dt )
{
    initDay();
    dayView->selectDate( dt );
    views->raiseWidget( dayView );
    dayView->setFocus();
    dayAction->setOn( TRUE );

    updateIcons();
}

void DateBook::initDay()
{
    if ( !dayView ) {
	dayView = new DayView( db, onMonday, views, "day view" ); // No tr
	views->addWidget( dayView, DAY );
	dayView->setDayStarts( startTime );
	connect( this, SIGNAL( newEvent() ),
		 dayView, SLOT( redraw() ) );
	connect( dayView, SIGNAL( newEvent() ),
		 this, SLOT( fileNew() ) );
	connect( dayView, SIGNAL( removeOccurrence( const Occurrence & ) ),
		 this, SLOT( removeOccurrence( const Occurrence & ) ) );
	connect( dayView, SIGNAL( editOccurrence( const Occurrence & ) ),
		 this, SLOT( editOccurrence( const Occurrence & ) ) );
	connect( dayView, SIGNAL( beamEvent( const PimEvent & ) ),
		 this, SLOT( beamEvent( const PimEvent & ) ) );
	connect( dayView, SIGNAL(newEvent(const QString &)),
		 this, SLOT(newEvent(const QString &)) );
	connect( dayView, SIGNAL(selectionChanged()),
		 this, SLOT(updateIcons()) );

	// qApp connections
	connect( qApp, SIGNAL(weekChanged(bool)),
		dayView, SLOT(setStartOnMonday(bool)) );
    }
}

void DateBook::initWeek()
{
    if ( !weekView ) {
	weekView = new WeekView( db, onMonday, views, "week view" ); // No tr
	weekView->setDayStarts( startTime );
	views->addWidget( weekView, WEEK );
	connect( weekView, SIGNAL( dateActivated( const QDate & ) ),
             this, SLOT( showDay( const QDate & ) ) );
	connect( this, SIGNAL( newEvent() ),
		 weekView, SLOT( redraw() ) );

	// qApp connections
	connect( qApp, SIGNAL(weekChanged(bool)),
		weekView, SLOT(setStartOnMonday(bool)) );
    }
}

void DateBook::initMonth()
{
    if ( !monthView ) {
	monthView = new MonthView( db, views, "month view", FALSE ); // No tr
	views->addWidget( monthView, MONTH );
	connect( monthView, SIGNAL( dateClicked( const QDate &) ),
             this, SLOT( showDay( const QDate &) ) );
	connect( this, SIGNAL( newEvent() ),
		 monthView, SLOT( updateOccurrences() ) );
	qApp->processEvents();
    }
}

void DateBook::loadSettings()
{
    {
	Config config( "qpe" );
	config.setGroup("Time");
	onMonday = config.readBoolEntry( "MONDAY" );
    }

    {
	Config config("DateBook");
	config.setGroup("Main");
	startTime = config.readNumEntry("startviewtime", 8);
	aPreset = config.readBoolEntry("alarmpreset");
	presetTime = config.readNumEntry("presettime");
    }
}

void DateBook::saveSettings()
{
    Config config( "qpe" );
    Config configDB( "DateBook" );
    configDB.setGroup( "Main" );
    configDB.writeEntry("startviewtime",startTime);
    configDB.writeEntry("alarmpreset",aPreset);
    configDB.writeEntry("presettime",presetTime);
}

void DateBook::appMessage(const QCString& msg, const QByteArray& data)
{
    bool needShow = FALSE;

    if ( msg == "alarm(QDateTime,int)" ) {
	QDataStream ds(data,IO_ReadOnly);
	QDateTime when; int warn;
	ds >> when >> warn;

	// check to make it's okay to continue,
	// this is the case that the time was set ahead, and
	// we are forced given a stale alarm...
	QDateTime current = QDateTime::currentDateTime();
	if ( current.time().hour() != when.time().hour()
	     && current.time().minute() != when.time().minute() )
	    return;

	bool ok;
	Occurrence item = db->getNextAlarm(when.addSecs(warn*60), &ok);
	if (ok) {
	    QString msg;
	    bool bSound = FALSE;
	    int stopTimer = 0;
	    msg += "<CENTER><B>" + item.event().description() + "</B>"
		+ "<BR>" + item.event().location() + "<BR>";

	    if (item.event().timeZone().isValid()) {
		QString tzText = item.event().timeZone().id();
		int i = tzText.find('/');
		tzText = tzText.mid( i + 1 );
		tzText = tzText.replace(QRegExp("_"), " ");

		msg += "<B>Time zone:</B> " + tzText + "<BR>";
	    }


	    msg += TimeString::localYMDHMS(item.event().start())
		+ (warn
			? " " + tr("(in %1 minutes)").arg(warn)
			: QString(""))
		+ "<BR>"
		+ item.event().notes() + "</CENTER>";
	    if ( item.event().alarmSound() != PimEvent::Silent ) {
		bSound = TRUE;
		Sound::soundAlarm();
		stopTimer = startTimer( 5000 );
	    }

	    QDialog dlg( this, 0, TRUE );
	    QVBoxLayout *vb = new QVBoxLayout( &dlg );
	    QScrollView *view = new QScrollView( &dlg, "scrollView");
	    view->setResizePolicy( QScrollView::AutoOneFit );
	    vb->addWidget( view );
	    QLabel *lblMsg = new QLabel( msg, &dlg );
	    view->addChild( lblMsg );
	    QPushButton *cmdOk = new QPushButton( tr("OK"), &dlg );
	    connect( cmdOk, SIGNAL(clicked()), &dlg, SLOT(accept()) );
	    vb->addWidget( cmdOk );

	    needShow = QPEApplication::execDialog(&dlg);

	    if ( bSound )
		killTimer( stopTimer );
	}
    } else if ( msg == "newEvent()" ) {
	if ( newEvent(QDateTime(),QDateTime(),QString::null,QString::null) )
	    needShow = TRUE;
    } else if ( msg == "receiveData(QString,QString)" ) {
	QDataStream stream(data,IO_ReadOnly);
	QString f,t;
	stream >> f >> t;
	if ( t.lower() == "text/x-vcalendar" )
	    if ( receiveFile(f) )
		needShow = TRUE;
	QFile::remove(f);
    } else if ( msg == "newEvent(QDateTime,QDateTime,QString,QString)" ) {
	QDateTime s,e;
	QString d,n;
	QDataStream ds(data,IO_ReadOnly);
	ds >> s >> e >> d >> n;
	if ( newEvent(s,e,d,n) )
	    needShow = TRUE;
    } else if ( msg == "raiseToday()" ) {
	bool visible=FALSE;
	if ( data.size() ) {
	    QDataStream ds(data,IO_ReadOnly);
	    int i;
	    ds >> i; // backdoor kludge
	    visible = i;
	}
	if ( visible )
	    nextView();
	else
	    viewToday();
	needShow = TRUE;
    } else if ( msg == "nextView()" ) {
	nextView();
	needShow = !!views->visibleWidget();
    } else if ( msg == "showEvent(QUuid)" ) {
	QUuid u;

	QDataStream ds( data, IO_ReadOnly );
	ds >> u;

	bool ok;
	Occurrence o = db->find(u, QDate::currentDate(), &ok);

	if (ok) {
	    viewDay( o.startInCurrentTZ().date() );
	    //dayView->selectDate( o.startInCurrentTZ().date() );
	    dayView->setCurrentItem(o);
	    needShow = TRUE;
	}
    } else if ( msg == "showEvent(QUuid,QDate)" ) {
	QUuid u;
	QDate date;

	QDataStream ds( data, IO_ReadOnly );
	ds >> u >> date;

	bool ok;
	Occurrence o = db->find(u, date, &ok);

	if (ok) {
	    viewDay( o.startInCurrentTZ().date() );
	    //dayView->selectDate( o.startInCurrentTZ().date() );
	    dayView->setCurrentItem(o);
	    needShow = TRUE;
	}
    }
    if ( needShow ) {
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
	//showMaximized();
#else
	//show();
#endif
	//raise();
	QPEApplication::setKeepRunning();
	//setActiveWindow();
    }
}

void DateBook::nextView()
{
    QWidget* cur = views->visibleWidget();
    if ( cur ) {
	if ( cur == dayView )
	    viewWeek();
	else if ( cur == weekView )
	    viewMonth();
	else if ( cur == monthView )
	    viewDay();
    }
}

void DateBook::reload()
{
    // reload isn't as meaningful anymore.
    //db->reload();} else
    db->reload();
    if ( dayAction->isOn() )
	viewDay();
    else if ( weekAction->isOn() )
	viewWeek();
    else if ( monthAction->isOn() )
	viewMonth();
    syncing = FALSE;
}

void DateBook::flush()
{
    db->flush();
    syncing = TRUE;
   // db->save();
    // neither is saving.
}

void DateBook::timerEvent( QTimerEvent *e )
{
    static int stop = 0;
    if ( stop < 10 ) {
	Sound::soundAlarm();
	stop++;
    } else {
	stop = 0;
	killTimer( e->timerId() );
    }
}

void DateBook::changeClock()
{
    // repaint the affected objects...
    if (dayView) dayView->redraw();
    if (weekView) weekView->redraw();
}

void DateBook::changeWeek( bool m )
{
    /* no need to redraw, each widget catches.  Do need to
       store though for widgets we haven't made yet */
    onMonday = m;
}

void DateBook::slotToday()
{
    // we need to view today
    QDate dt = QDate::currentDate();
    if ( views->visibleWidget() == dayView ) {
	showDay( dt );
    } else if (views->visibleWidget() == weekView) {
	weekView->selectDate( dt );
    } else if (views->visibleWidget() == monthView){
	monthView->setDate( dt );
    }
}

void DateBook::closeEvent( QCloseEvent *e )
{
    slotToday();
    if(syncing) {
	/* no need to save, did that at flush */
	e->accept();
	return;
    }

    // save settings will generate it's own error messages, no
    // need to do checking ourselves.
    saveSettings();
    //if ( db->save() )
    e->accept();
#if 0
    else {
	if ( QMessageBox::critical( this, tr( "Out of space" ),
				    tr("Calendar was unable to save\n"
				       "your changes.\n"
				       "Free up some space and try again.\n"
				       "\nQuit anyway?"),
				    QMessageBox::Yes|QMessageBox::Escape,
				    QMessageBox::No|QMessageBox::Default )
	     != QMessageBox::No )
	    e->accept();
	else
	    e->ignore();
    }
#endif
}

void DateBook::newEvent( const QString &description )
{
    QDateTime current = QDateTime::currentDateTime();
    QDateTime start=current, end=current;

    int mod = QTime(0,0,0).secsTo(current.time()) % 900;
    if (mod != 0)  {
	mod = 900 - mod;
	current.setTime( current.time().addSecs( mod ) );
    }

    start.setTime(current.time());
    end.setTime(start.time().addSecs(3600));

    newEvent(start,end,description,QString::null);
}

bool DateBook::newEvent(const QDateTime& dstart,const QDateTime& dend,const QString& description,const QString& notes)
{
    if (syncing) {
	QMessageBox::warning( this, tr("Calendar"),
	                      tr( "Can not edit data, currently syncing") );
	return FALSE;
    }

    QDateTime start=dstart, end=dend;
    QDateTime current = QDateTime::currentDateTime();
    bool snull = start.date().isNull();
    bool enull = end.date().isNull();

    if ( views->visibleWidget() ) {
	if ( views->visibleWidget() == dayView ) {
	    dayView->selectedDates( start, end );
	} else if ( views->visibleWidget() == monthView ) {
	    start.setDate( monthView->selectedDate() );
	} else if ( views->visibleWidget() == weekView ) {
	    start.setDate( weekView->currentDate() );
	}
    }
    if ( start.date().isNull() )
	start.setDate( current.date() );
    if ( end.date().isNull() )
	end.setDate( start.date() );
    if ( (snull && end.time().isNull()) || !start.time().isValid() ) {
	// We get to here from a key pressed in the Day View
	// So we can assume some things.  We want the string
	// passed in to be part of the description.
	// move current to the next fifteen minutes
	int mod = QTime(0,0,0).secsTo(current.time()) % 900;
	if (mod != 0)  {
	    mod = 900 - mod;
	    current.setTime( current.time().addSecs( mod ) );
	}

	// default start
	start.setTime(current.time());
    }
    if ( (enull && end.time().isNull()) || !end.time().isValid() ) {
	// default end
	end.setTime(start.time().addSecs(3600));
    }


    // argh!  This really needs to be encapsulated in a class
    // or function.
    QDialog newDlg( this, 0, TRUE );
    newDlg.setCaption( DateEntryBase::tr("New Event") );
    DateEntry *e;
    QVBoxLayout *vb = new QVBoxLayout( &newDlg );
    QScrollView *sv = new QScrollView( &newDlg );
    sv->setResizePolicy( QScrollView::AutoOneFit );
    sv->setFrameStyle( QFrame::NoFrame );
    sv->setHScrollBarMode( QScrollView::AlwaysOff );
    vb->addWidget( sv );

    PimEvent ev;
    ev.setDescription(  description );
    // When the new gui comes in, change this...
    ev.setLocation( "" );
    ev.setStart( start );
    ev.setEnd( end );
    ev.setNotes( notes );

    VScrollBox *vsb = new VScrollBox( &newDlg );
    e = new DateEntry( onMonday, ev, vsb );
    e->setAlarmEnabled( aPreset, presetTime, PimEvent::Loud );
    sv->addChild( vsb );
    qDebug( "newDlg sizeHint(): %d,%d", newDlg.sizeHint().width(), newDlg.sizeHint().height() );
    qDebug( "e minimumsize: %d, %d", e->minimumSize().width(), e->minimumSize().height() );
    while (QPEApplication::execDialog(&newDlg)) {
	ev = e->event();
	//ev.assignUid(); // um, don't know if we can drop this or not
	QString error = checkEvent( ev );
	if ( !error.isNull() ) {
	    if ( QMessageBox::warning( this, tr("Error!"),
				       error, tr("Fix it"), tr("Continue"), 0, 0, 1 ) == 0 )
		continue;
	}
	QUuid id = db->addEvent( ev );
	emit newEvent();
	if ( views->visibleWidget() == dayView ) {
	    dayView->clearSelectedDates();
	    bool ok;
	    PimEvent e = db->find( id, &ok );
	    if ( ok )
		dayView->setCurrentEvent( e );
	}
	return TRUE;
    }
    if ( views->visibleWidget() == dayView )
	dayView->clearSelectedDates();
    return FALSE;
}

void DateBook::setDocument( const QString &filename )
{
    DocLnk doc(filename);
    if ( doc.isValid() )
	receiveFile(doc.file());
    else
	receiveFile(filename);
}

bool DateBook::receiveFile( const QString &filename )
{
    QValueList<PimEvent> tl = PimEvent::readVCalendar( filename );

    QString msg = tr("<P>%1 new events.<p>Do you want to add them to your Calendar?").
	arg(tl.count());

    if ( QMessageBox::information(this, tr("New Events"),
	    msg, QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Ok ) {
	QDateTime from,to;
	for( QValueList<PimEvent>::Iterator it = tl.begin(); it != tl.end(); ++it ) {
	    if ( from.isNull() || (*it).start() < from )
		from = (*it).start();
	    if ( to.isNull() || (*it).end() < to )
		to = (*it).end();
	    db->addEvent( *it );
	}

	// Change view to a sensible one...
	if ( from.date() == to.date() ) {
	    viewDay(from.date());
	} else {
	    initWeek();
	    int fw,fy,tw,ty;
	    weekView->calcWeek(from.date(), fw, fy);
	    weekView->calcWeek(to.date(), tw, ty);
	    if ( fw == tw && fy == ty ) {
		viewWeek(from.date());
	    } else if ( from.date().month() == to.date().month()
		    && from.date().year() == to.date().year() ) {
		viewMonth(from.date());
	    } else {
		viewDay(from.date());
	    }
	}

	emit newEvent();
	return TRUE;
    }
    return FALSE;
}

static const char * beamfile = "/tmp/obex/event.vcs";

void DateBook::beamEvent( const PimEvent &e )
{
#ifdef Q_WS_QWS
    unlink( beamfile ); // delete if exists

#if defined (Q_OS_WIN32)
    QDir d;
    d.mkdir("/tmp/obex/");
#else
    ::mkdir("/tmp/obex/", 0755);
#endif
    PimEvent::writeVCalendar( beamfile, e );
    Ir *ir = new Ir( this );
    connect( ir, SIGNAL( done( Ir * ) ), this, SLOT( beamDone( Ir * ) ) );
    QString description = e.description();
    ir->send( beamfile, description, "text/x-vCalendar" );
#endif
}

void DateBook::beamDone( Ir *ir )
{
#ifdef Q_WS_QWS
    delete ir;
    unlink( beamfile );
#endif
}


void DateBook::slotFind()
{
    // move it to the day view...
    viewDay();
    FindDialog frmFind( "Calendar", this );
    frmFind.setUseDate( true );
    frmFind.setDate( currentDate() );
    QObject::connect( &frmFind,
                      SIGNAL(signalFindClicked(const QString&, const QDate&,
					       bool, bool, int)),
		      this,
		      SLOT(slotDoFind(const QString&, const QDate&,
				      bool, bool, int)) );
    QObject::connect( this,
		      SIGNAL(signalNotFound()),
		      &frmFind,
		      SLOT(slotNotFound()) );
    QObject::connect( this,
		      SIGNAL(signalWrapAround()),
		      &frmFind,
		      SLOT(slotWrapAround()) );
    frmFind.exec();
    inSearch = false;
}

bool catComp( QArray<int> cats, int category )
{
    bool returnMe;
    int i,
	count;

    count = int(cats.count());
    returnMe = false;
    if ( (category == -1 && count == 0) || category == -2 )
	returnMe = true;
    else {
	for ( i = 0; i < count; i++ ) {
	    if ( category == cats[i] ) {
		returnMe = true;
		break;
	    }
	}
    }
    return returnMe;
}

void DateBook::slotDoFind( const QString& txt, const QDate &dt,
			   bool caseSensitive, bool searchForward,
			   int category )
{
    bool ok;
    QRegExp r(txt);
    r.setCaseSensitive(caseSensitive);

    Occurrence o = db->find(r, category, dt,
	    searchForward, &ok);

    if ( ok ) {
	dayView->selectDate( o.startInCurrentTZ().date() );
	dayView->setCurrentItem(o);
    }
}
