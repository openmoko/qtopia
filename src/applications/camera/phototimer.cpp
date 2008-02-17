/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include "phototimer.h"
#include "minsecspinbox.h"
#include "noeditspinbox.h"

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLCDNumber>

// Qtopia includes
#include <QAnalogClock>
#include <QtopiaApplication>

PhotoTimer::PhotoTimer( int timeout,
                        int number,
                        int interval,
                        QWidget* parent,
                        Qt::WFlags f )
:   QWidget( parent, f ),
    mTimeout( timeout ),
    mNumber( number ),
    mInterval( interval ),
    mClock( 0 )
{
    QGridLayout* layout = new QGridLayout( this );

    mClock = new QAnalogClock( this );
    mClock->display( QTime( 0, mTimeout, mTimeout ) );
    mClock->setFace( QPixmap( ":image/clock/background" ) );
    layout->addWidget( mClock, 0, 0 );
    setLayout( layout );

    setMinimumSize( 70, 70 );

    QTimer* timer = new QTimer( this );
    timer->setInterval( 1000 );
    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );
    timer->start();
}

void PhotoTimer::timeout()
{
    if ( mTimeout <= 0 ) {
        emit( takePhoto() );
        if ( mNumber <= 1 ) {
            deleteLater();
        } else {
            mTimeout = mInterval;
            --mNumber;
        }
    } else {
        --mTimeout;
    }

    mClock->display( QTime( 0, mTimeout, mTimeout ) );
}

PhotoTimerDialog::PhotoTimerDialog( QWidget* parent, Qt::WFlags f )
:   QDialog( parent, f ),
    mTimeout( 5 ),
    mNumber( 1 ),
    mInterval( 1 ),
    mIntervalSpin( 0 )
{
    setWindowTitle( tr( "Photo Timer" ) );
    setModal( true);

    QGridLayout* layout = new QGridLayout( this );

    // Add labels
    layout->addWidget( new QLabel( tr( "Timeout" ) ), 0, 0 );
    QSpinBox* timeout = new CameraMinSecSpinBox( this );
    timeout->setMinimum( 1 );
    timeout->setMaximum( 120 );
    timeout->setValue( mTimeout );
    layout->addWidget( timeout, 0, 1 );
    connect( timeout,
             SIGNAL(valueChanged(int)),
             this,
             SLOT(timeoutChanged(int)) );

    layout->addWidget( new QLabel( tr( "Photos" ) ), 1, 0 );
    QSpinBox* number = new NoEditSpinBox(this);
    number->setMinimum( 1 );
    number->setMaximum( 50 );
    number->setValue( mNumber );
    layout->addWidget( number, 1, 1 );
    connect( number,
             SIGNAL(valueChanged(int)),
             this,
             SLOT(numberChanged(int)) );

    layout->addWidget( new QLabel( tr( "Interval" ) ), 2, 0 );
    mIntervalSpin = new CameraMinSecSpinBox( this );
    mIntervalSpin->setMinimum( 1 );
    mIntervalSpin->setMaximum( 120 );
    mIntervalSpin->setValue( mInterval );
    mIntervalSpin->setEnabled( false );
    layout->addWidget( mIntervalSpin, 2, 1 );
    connect( mIntervalSpin,
             SIGNAL(valueChanged(int)),
             this,
             SLOT(intervalChanged(int)) );

    setLayout( layout );
    
    QtopiaApplication::setInputMethodHint(timeout,QtopiaApplication::AlwaysOff);
    QtopiaApplication::setInputMethodHint(number,QtopiaApplication::AlwaysOff);
    QtopiaApplication::setInputMethodHint(mIntervalSpin,QtopiaApplication::AlwaysOff);

}

int PhotoTimerDialog::timeout() const
{
    return mTimeout;
}

int PhotoTimerDialog::number() const
{
    return mNumber;
}

int PhotoTimerDialog::interval() const
{
    return mInterval;
}

void PhotoTimerDialog::timeoutChanged( int timeout )
{
    mTimeout = timeout;
}

void PhotoTimerDialog::numberChanged( int number )
{
    mNumber = number;
    if ( mNumber > 1 )
        mIntervalSpin->setEnabled( true );
    else
        mIntervalSpin->setEnabled( false );
}

void PhotoTimerDialog::intervalChanged( int interval )
{
    mInterval = interval;
}



