/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifdef QT_NO_SXE
#include <QTest>
QTEST_NOOP_MAIN
#else

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>

#define TEST_FAR_ENFORCER
#include "qtransportauth_qws_p.h"
#include <qdebug.h>
#include <qtopianamespace.h>
#include <qprocess.h>
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>

QString lastLogMessage;

//TESTED_CLASS=FAREnforcer
//TESTED_FILES=

void syslog (int /*pri*/, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *log_message;
    vasprintf(&log_message, fmt, args);
    lastLogMessage = QString::fromLatin1(log_message);
    free(log_message);
}

/*
  The tst_FAREnforcer class performs a number of unit tests
  on the Global Authentication Rate Enforcer to ensure
  correct operation.
*/

/*
  Implementation Detail:

  The FAREnforcer code is found in qtransportauth_qws_p.h
  in /src/gui/embedded in qt code.  For simplicity and due
  to the fact that the FAREnforcer is "qtopia only" code
  it has been placed in the qtopia testing source tree.

  Executing these tests is a little slow since a delay
  has been introduced for every log check.  The delay
  was introduced to avoid race conditions where the log check
  happens before the log is updated.
*/
class tst_FAREnforcer: public QObject
{
    Q_OBJECT

public:
    tst_FAREnforcer();

private slots:
    void initTestCase();

    void exceedRate();
    void exceedRate_data();

    void checkBoundaryOverRate();
    void checkBoundaryOverRate_data();

    void checkBoundaryUnderRate();
    void checkBoundaryUnderRate_data();

    void delayedExceedRate();
    void delayedExceedRate_data();

    void underRate();
    void underRate_data();

private:
    bool checkForLogEntry();
    void processData();
    const char * dataTag( QString str, int i )
    {
        return qPrintable( str + ": " +  QString::number(i) );
    }
    FAREnforcer *g;
};

/*?
  \internal
  constructor
*/
tst_FAREnforcer::tst_FAREnforcer():QObject()
{
}

/*?
  \internal
  Initial setup
*/
void tst_FAREnforcer::initTestCase()
{
    syslog( LOG_ERR | LOG_LOCAL6, "%s", "" );
    g = FAREnforcer::getInstance();
}

/*?
  \internal
  Exceed the FAR and ensure the  FAR Exceeded log entry is generated.
  Exceed the FAR  again with another set of to show authentications
  are still allowed, (must allow authentication in the event the
  user re-enables and runs an application), and to show that the FAR
  Enforcer still behaves correctly.
*/

void tst_FAREnforcer::exceedRate()
{
    static bool firstInvokation = true;
    if ( firstInvokation )
    {
        g->reset();
        firstInvokation = false;
    }

    QFETCH(QDateTime, Input);
    QFETCH(bool, Output );
    
    g->logAuthAttempt( Input );
    
    QCOMPARE( checkForLogEntry(), Output );
}

void tst_FAREnforcer::exceedRate_data()
{
    QTest::addColumn<QDateTime>("Input");
    QTest::addColumn<bool>("Output");
    
    QDateTime dt = QDateTime::currentDateTime();
    
    for ( int i = 0; i < FAREnforcer::minutelyRate; i++ )
        QTest::newRow( dataTag("First pass",i) ) << dt << false; 
   
    QTest::newRow("First pass exceed") << dt << true;

    for ( int i = 0; i < FAREnforcer::minutelyRate; i++ )
    {
        dt.addSecs( 1 );
        QTest::newRow( dataTag("Second pass",i) ) << dt << false;
    }
        
    dt.addSecs( 1 );
    QTest::newRow("Second pass exceed") << dt << true; 
}

/*?
  \internal
   exceed the FAR at the boundary condition where the (hourlyRate + 1)th
   authentication happens exactly a hour after the 1st authentication
   (and at the same time as the hourlyRate-th authentication )
*/
void tst_FAREnforcer::checkBoundaryOverRate()
{
    static bool firstInvokation = true;
    if ( firstInvokation )
    {
        g->reset();
        firstInvokation = false;
    }
    processData();
}

void tst_FAREnforcer::checkBoundaryOverRate_data()
{
    QTest::addColumn<QDateTime>("Input");
    QTest::addColumn<bool>("Output");
    
    QDateTime dt = QDateTime::currentDateTime();
    
    for ( int i = 0; i < FAREnforcer::minutelyRate - 1; i++ )
        QTest::newRow( dataTag("before boundary", i) ) << dt.addSecs( FAREnforcer::minute * i / FAREnforcer::minutelyRate ) << false;

    dt = dt.addSecs( FAREnforcer::minute );

    QTest::newRow("At boundary") << dt << false;
    QTest::newRow("Over rate") << dt << true;
}

/*?
  \internal
  boundary condition test where the (hourlyRate + 1)th authentication
  happens just over an hour after the 1st authentication
  (and at 1 second after the hourlyRate-th authentication )
*/
void tst_FAREnforcer::checkBoundaryUnderRate()
{
    static bool firstInvokation = true;
    if ( firstInvokation )
    {
        g->reset();
        firstInvokation = false;
    }
    processData();
}

void tst_FAREnforcer::checkBoundaryUnderRate_data()
{
    QTest::addColumn<QDateTime>("Input");
    QTest::addColumn<bool>("Output");
    
    QDateTime dt = QDateTime::currentDateTime();

    for ( int i = 0; i < FAREnforcer::minutelyRate - 1; i++ )
        QTest::newRow( dataTag("before boundary", i) ) << dt.addSecs( FAREnforcer::minute * i / FAREnforcer::minutelyRate ) << false;

    dt = dt.addSecs( FAREnforcer::minute );
    QTest::newRow("At boundary") << dt << false;

    dt = dt.addSecs( 1 );
    QTest::newRow("Under rate") << dt << false;
}

/*?
  \internal
  Exceed the FAR but at a delayed time.  Initially almost exceed the
  FAR, then use a set of valid authentication times, then exceed the
  FAR rate.
*/  
void tst_FAREnforcer::delayedExceedRate()
{
    static bool firstInvokation = true;
    if ( firstInvokation )
    {
        g->reset();
        firstInvokation = false;
    }
    
    processData(); 
}

void tst_FAREnforcer::delayedExceedRate_data()
{
    QTest::addColumn<QDateTime>("Input");
    QTest::addColumn<bool>("Output");

    QDateTime dt = QDateTime::currentDateTime();

    //feed in identical times up to limit 
    for ( int i = 0; i < FAREnforcer::minutelyRate; i++ )
        QTest::newRow( dataTag("First pass, hit rate limit",i) ) << dt << false;

    dt = dt.addSecs( FAREnforcer::minute + 1 );

    // feed in a new set of valid authentication times
    for ( int i = 0; i < FAREnforcer::minutelyRate; i++ )
        QTest::newRow( dataTag("Second pass, hit rate limit again",i) ) 
                << dt.addSecs( FAREnforcer::minute * i / FAREnforcer::minutelyRate ) << false;

    dt = dt.addSecs( FAREnforcer::minute * 2 );
    // feed in another set of authentication times but exceed the FAR by a factor of 2
    for ( int i = 0; i < FAREnforcer::minutelyRate + 1; i++ )
    {
        //authenticate at twice the hourly rate
        QTest::newRow( dataTag("Third pass, exceed limit",i) ) << dt.addSecs( FAREnforcer::minute * i / (FAREnforcer::minutelyRate * 2) )  
                                                            << (( i < FAREnforcer::minutelyRate )? false: true);
    }
}

/*?
  \internal
  Maintain authentication attempts at exactly the allowable rate
*/
void tst_FAREnforcer::underRate()
{
    static bool firstInvokation = true;
    if ( firstInvokation )
    {
        g->reset();
        firstInvokation = false;
    }
    processData(); 
}

void tst_FAREnforcer::underRate_data()
{
    QTest::addColumn<QDateTime>("Input");
    QTest::addColumn<bool>("Output");
    
    QDateTime dt = QDateTime::currentDateTime();
    
    // for "5 minutes" maintain rate just at allowed limit
    for ( int i = 0; i < FAREnforcer::minute * 5; i++ )
    {   
        if ( i % FAREnforcer::minutelyRate == 0 &&  i != 0  )
            dt = dt.addSecs(1);

        QTest::newRow( dataTag("before boundary",i ) ) << dt.addSecs( FAREnforcer::minute * i / FAREnforcer::minutelyRate ) << false;
    }
}

/*?
  \internal
  Checks the last line of the sxe log for the existance of the FAR Exceeded 
  log entry.  A bool is returned whose value depends on whether the
  entry was expected to be found or not
*/
bool tst_FAREnforcer::checkForLogEntry()
{
    QString str(lastLogMessage);

    //create arbitrary log entry so checkSxeLog isn't "fooled" when it is called again
    syslog( LOG_ERR | LOG_LOCAL6, "%s", "" );

    //check for the FAR Exceeded log entry
    if ( str.contains( FAREnforcer::SxeTag )
            /* && str.contains( QString::number(getpid()) ) */      // This functionality seems to have been removed.
            && str.contains(FAREnforcer::FARMessage) )
    {
        return true;
    }
    return false;
}

void tst_FAREnforcer::processData()
{
    QFETCH(QDateTime, Input);
    QFETCH(bool, Output );

    g->logAuthAttempt( Input );

    QCOMPARE( checkForLogEntry(), Output );
}

QTEST_APP_MAIN(tst_FAREnforcer, QCoreApplication)
#include "tst_farenforcer.moc"

#endif

