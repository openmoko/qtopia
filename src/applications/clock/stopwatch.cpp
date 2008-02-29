/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "stopwatch.h"

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qsettings.h>
#include <qtimestring.h>
#include <qsoftmenubar.h>


#include <qlcdnumber.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qpainter.h>
#include <qtabwidget.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qevent.h>
#include <QKeySequence>
#include <QAnalogClock>


static const int sw_prec = 2;
static const int magic_daily = 2292922;
static const int magic_countdown = 2292923;

static void toggleScreenSaver( bool on )
{
    QtopiaApplication::setPowerConstraint(on ? QtopiaApplication::Enable : QtopiaApplication::DisableSuspend);
}

StopWatch::StopWatch( QWidget * parent, Qt::WFlags f )
    : QWidget( parent, f ), swatch_splitms(99) // No tr
{
    setupUi(this);
    swLayout = 0;

    analogStopwatch = new QAnalogClock( swFrame );
    stopwatchLcd = new QLCDNumber( swFrame );
    stopwatchLcd->setFrameStyle( QFrame::NoFrame );
    stopwatchLcd->setSegmentStyle( QLCDNumber::Flat );
    stopwatchLcd->setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred) );

    connect( stopStart, SIGNAL(pressed()), SLOT(stopStartStopWatch()) );
    connect( reset, SIGNAL(pressed()), SLOT(resetStopWatch()) );

#ifdef QTOPIA_PHONE
    if (!Qtopia::mousePreferred()) {
        int iconSz = style()->pixelMetric(QStyle::PM_SmallIconSize);
        stopStart->setIconSize(QSize(iconSz, iconSz));
        stopStart->setIcon( QIcon( ":icon/select" ) );
        stopStart->setText( " " + stopStart->text() );
        //stopStart->setShortcut(QKeySequence(Qt::Key_Select));
        reset->setText("# " + reset->text());
        reset->setShortcut(QKeySequence(Qt::Key_NumberSign));

        // The buttons DO need keyboard focus, otherwise it is not possible to operate them
        // on a non-touchscreen phone.
        //stopStart->setFocusPolicy(Qt::NoFocus);
        //reset->setFocusPolicy(Qt::NoFocus);

        // Took this next line out because it doesn't apply when the Reset button has the focus.
        // There's no longer any need for it anyway, because we have re-introduced focus.
        //QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Select );

        // Make sure the stop/start button is the one that has focus at first.
        stopStart->setFocus(Qt::OtherFocusReason);
    }
#endif

    t = new QTimer( this );
    connect( t, SIGNAL(timeout()), SLOT(updateClock()) );

    connect( qApp, SIGNAL(timeChanged()), SLOT(updateClock()) );

    swatch_running = false;
    swatch_totalms = 0;
    swatch_currLap = 0;
    swatch_dispLap = 0;
    stopwatchLcd->setNumDigits( 8+1+sw_prec );

    QVBoxLayout *lvb = new QVBoxLayout( lapFrame );
    lvb->setMargin(0);

    nextLapBtn = new QToolButton( lapFrame );
    nextLapBtn->setArrowType(Qt::UpArrow);
    connect( nextLapBtn, SIGNAL(clicked()), this, SLOT(nextLap()) );
    nextLapBtn->setShortcut( QKeySequence(Qt::Key_Up) );
    lvb->addWidget( nextLapBtn );

    prevLapBtn = new QToolButton( lapFrame );
    prevLapBtn->setArrowType(Qt::DownArrow);
    connect( prevLapBtn, SIGNAL(clicked()), this, SLOT(prevLap()) );
    prevLapBtn->setShortcut( QKeySequence(Qt::Key_Down) );
    prevLapBtn->setMinimumWidth( 15 );
    lvb->addWidget( prevLapBtn );

    prevLapBtn->setEnabled( false );
    nextLapBtn->setEnabled( false );

#ifdef QTOPIA_PHONE
    if (!Qtopia::mousePreferred()) {
        // The buttons DO need keyboard focus, otherwise it is not possible to operate them
        // on a non-touchscreen phone.
        //nextLapBtn->setFocusPolicy ( Qt::NoFocus );
        //prevLapBtn->setFocusPolicy ( Qt::NoFocus );
    }
#endif

    reset->setEnabled( false );

    lapLcd->setNumDigits( 8+1+sw_prec );
    splitLcd->setNumDigits( 8+1+sw_prec );

    lapNumLcd->display( 1 );

    lapTimer = new QTimer( this );
    lapTimer->setSingleShot(true);
    connect( lapTimer, SIGNAL(timeout()), this, SLOT(lapTimeout()) );

    for (int s = 0; s < swatch_splitms.count(); s++ )
        swatch_splitms[s] = 0;

    connect( qApp, SIGNAL(clockChanged(bool)), this, SLOT(changeClock(bool)) );

    swFrame->installEventFilter( this );
    updateClock();
}

StopWatch::~StopWatch()
{
    toggleScreenSaver( true );
}

void StopWatch::updateClock()
{
    int totalms = swatch_totalms;
    if ( swatch_running )
        totalms += swatch_start.elapsed();
    setSwatchLcd( stopwatchLcd, totalms, !swatch_running );
    QTime swatch_time = QTime(0,0,0).addMSecs(totalms);
    analogStopwatch->display( swatch_time );
    if ( swatch_dispLap == swatch_currLap ) {
        swatch_splitms[swatch_currLap] = swatch_totalms;
        if ( swatch_running )
            swatch_splitms[swatch_currLap] += swatch_start.elapsed();
        updateLap();
    }
}

void StopWatch::changeClock( bool )
{
    updateClock();
}

void StopWatch::stopStartStopWatch()
{
    if ( swatch_running ) {
        swatch_totalms += swatch_start.elapsed();
        swatch_splitms[swatch_currLap] = swatch_totalms;
        stopStart->setText( tr("Start") );
        reset->setText( tr("Reset") );
#ifdef QTOPIA_PHONE
        if (!Qtopia::mousePreferred()) {
            stopStart->setIcon( QIcon( ":icon/select" ) );
            stopStart->setText( " " + stopStart->text() );
            //stopStart->setShortcut(Qt::Key_Select);
            reset->setText( "# " + reset->text() );
            reset->setShortcut(Qt::Key_NumberSign);
        }
#endif
        reset->setEnabled( true );
        t->stop();
        swatch_running = false;
        toggleScreenSaver( true );
        updateClock();
    } else {
        swatch_start.start();
        stopStart->setText( tr("Stop") );
        reset->setText( tr("Lap/Split") );
#ifdef QTOPIA_PHONE
        if (!Qtopia::mousePreferred()) {
            stopStart->setIcon( QIcon( ":icon/select" ) );
            stopStart->setText( " " + stopStart->text() );
//            stopStart->setShortcut(Qt::Key_Select);
            reset->setText( "# " + reset->text() );
            reset->setShortcut(Qt::Key_NumberSign);
        }
#endif
        reset->setEnabled( swatch_currLap < 98 );
        t->start( 1000 );
        swatch_running = true;
        // disable screensaver while stop watch is running
        toggleScreenSaver( false );
    }
    swatch_dispLap = swatch_currLap;
    updateLap();
    prevLapBtn->setEnabled( swatch_dispLap );
    nextLapBtn->setEnabled( swatch_dispLap < swatch_currLap );
#ifndef QTOPIA_PHONE
    stopStart->setShortcut( QKeySequence(Qt::Key_Return) );
#endif
}

void StopWatch::resetStopWatch()
{
    if ( swatch_running ) {
        swatch_splitms[swatch_currLap] = swatch_totalms+swatch_start.elapsed();
        swatch_dispLap = swatch_currLap;
        if ( swatch_currLap < 98 )  // allow up to 99 laps
            swatch_currLap++;
        reset->setEnabled( swatch_currLap < 98 );
        updateLap();
        lapTimer->start( 2000 );
    } else {
        swatch_start.start();
        swatch_totalms = 0;
        swatch_currLap = 0;
        swatch_dispLap = 0;
        for ( int i = 0; i < swatch_splitms.count(); i++ )
            swatch_splitms[i] = 0;
        updateLap();
        updateClock();
        reset->setText( tr("Lap/Split") );
        reset->setEnabled( false );
    }
    prevLapBtn->setEnabled( swatch_dispLap );
    nextLapBtn->setEnabled( swatch_dispLap < swatch_currLap );
}

void StopWatch::prevLap()
{
    if ( swatch_dispLap > 0 ) {
        swatch_dispLap--;
        updateLap();
        prevLapBtn->setEnabled( swatch_dispLap );
        nextLapBtn->setEnabled( swatch_dispLap < swatch_currLap );
    }
}

void StopWatch::nextLap()
{
    if ( swatch_dispLap < swatch_currLap ) {
        swatch_dispLap++;
        updateLap();
        prevLapBtn->setEnabled( swatch_dispLap );
        nextLapBtn->setEnabled( swatch_dispLap < swatch_currLap );
    }
}

void StopWatch::lapTimeout()
{
    swatch_dispLap = swatch_currLap;
    updateLap();
    prevLapBtn->setEnabled( swatch_dispLap );
    nextLapBtn->setEnabled( swatch_dispLap < swatch_currLap );
}

void StopWatch::updateLap()
{
    if ( swatch_running && swatch_currLap == swatch_dispLap ) {
        swatch_splitms[swatch_currLap] = swatch_totalms;
        swatch_splitms[swatch_currLap] += swatch_start.elapsed();
    }
    int split = swatch_splitms[swatch_dispLap];
    int lap;
    if ( swatch_dispLap > 0 )
        lap = swatch_splitms[swatch_dispLap] - swatch_splitms[swatch_dispLap-1];
    else
        lap = swatch_splitms[swatch_dispLap];

    lapNumLcd->display( swatch_dispLap+1 );
    bool showMs = !swatch_running || swatch_dispLap!=swatch_currLap;
    setSwatchLcd( lapLcd, lap, showMs );
    setSwatchLcd( splitLcd, split, showMs );
}

void StopWatch::setSwatchLcd( QLCDNumber *lcd, int ms, bool showMs )
{
    QTime swatch_time = QTime(0,0,0).addMSecs(ms);
    QString d = showMs ? QString::number(ms%1000+1000) : QString("    ");
    QString lcdtext = swatch_time.toString() + "." + d.right(3).left(sw_prec);
    lcd->display( lcdtext );
    lcd->repaint();
}

bool StopWatch::eventFilter( QObject *o, QEvent *e )
{
    if ( o == swFrame && e->type() == QEvent::Resize ) {
        QResizeEvent *re = (QResizeEvent *)e;
        delete swLayout;
        if ( re->size().height() < 80 || re->size().height()*3 < re->size().width() )
            swLayout = new QHBoxLayout( swFrame );
        else
            swLayout = new QVBoxLayout( swFrame );
        swLayout->setMargin(0);
        swLayout->addWidget( analogStopwatch );
        swLayout->addWidget( stopwatchLcd );
        swLayout->activate();
    }

    return false;
}

void StopWatch::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    updateClock();
    /*
      // We don't need to do this, because we are giving the stop/start button the focus in the ctor.
#ifdef QTOPIA_PHONE
    if (!Qtopia::mousePreferred()) {
           setFocus();
    }
#endif
    */
}

