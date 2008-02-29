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

#include "performancetest.h"
#include <QtopiaChannel>
#include <QtopiaIpcEnvelope>
#include <unistd.h>
#include <stdlib.h>
#include <Qtopia>
#include <QTimer>

/*!
  \class QAPerformanceTest
  \ingroup QtopiaServer::Task
  \brief The QAPerformanceTest class provides performance testing instrumentation to the Qtopia Server.

  On receipt of a "runPerftest()" message on the "QPE/PerformanceTest" this
  task starts and stops a series of applications inside the qpe event loop
  in order to allow collection of performance data for application startup
  times.  Following the test, the server exists.

  Alternatively, sending the "runPerfTestNoQuit()" message performs the same
  test, but the server continues running.
 */

/*! \internal
    \fn void QAPerformanceTest::shutdown( QtopiaServerApplication::ShutdownType )
*/

/*! \internal */
QAPerformanceTest::QAPerformanceTest()
: m_willQuit(false)
{
    QtopiaChannel *channel = new QtopiaChannel("QPE/PerformanceTest", this);
    connect(channel, SIGNAL(received(const QString&,const QByteArray&)),
            this, SLOT(perfMsg(const QString&,const QByteArray&)));
}

/*! \internal */
void QAPerformanceTest::perfMsg(const QString &msg, const QByteArray &)
{
    if(msg == "runPerfTest()") {
        m_willQuit = true;
        m_closeApps = true;
        runPerformanceTest();
    } else if(msg == "runPerfTestNoQuit()") {
        m_willQuit = false;
        m_closeApps = true;
        runPerformanceTest();
    } else if(msg == "runAllApps()") {
        m_willQuit = false;
        m_closeApps = false;
        runPerformanceTest();
    }
}

/*! \internal */
void QAPerformanceTest::runPerformanceTest()
{
    static int step = 0;
    static const int stepTime = 7000;
    static const char * const testApps[] = {
        // Applications
        "calculator",
        "datebook",          // Calendar
        //"callhistory",
        "camera",
        "clock",
        "addressbook",       // Contacts
        "helpbrowser",       // Help
        "qtmail",            // Messages
        "textedit",          // Notes
        "photoedit",         // Pictures
        "simapp",            // SIM Applications
        "sysinfo",           // System Info
        "todolist",          // Tasks
        "mediarecorder",     // Voice Notes
        "worldtime",

        // Settings
        "appearance",
        "beaming",
#ifdef QTOPIA_PHONE
        "callforwarding",
        "phonenetworks",     // Call Networks
        "phonesettings",     // Call Options
#endif
        "systemtime",        // Date/Time
        "hwsettings",        // Handwriting
        "netsetup",          // Internet
        "language",
        "light-and-power",   // Power Management
        "sipsettings",       // VoIP
        "logging",
#ifdef QTOPIA_PHONE
        "profileedit",       // Profiles
        "speeddial",
#endif
        "words",
#ifndef QTOPIA_PHONE
        "appservices",       // Application Services
        "buttoneditor",      // Buttons
        "launchersettings",  // Launcher
        "rotation",
        "packagemanager",    // Software Packages
#endif

        // Games
        "fifteen",
        "minesweep",
        "qasteroids",
        "snake", 

        // Test Apps
        "emptyapp" };

    static const int numTestApps = sizeof(testApps)/sizeof(char *);
/*
    if (step >= numTestApps * 2)
    {
        if(m_willQuit) {
            // Test is finished - shutdown Qtopia
            qLog(Performance) << "QtopiaServer : " << "Shutting down Qtopia : "
                              << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
            //::exit(0);
            emit shutdown( QtopiaServerApplication::RebootSystem );
        }

        return;
    }

    else if (step % 2 == 0)
    {
        // Even step - start the next application
        qLog(Performance) << "Application : " << "Launching" << testApps[step/2] << " : "
                          << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
        Qtopia::execute(testApps[step/2]);
    }
    else
    {
        if (m_closeApps == true) {
            // Odd step - stop the current application
            qLog(Performance) << "Application : " << "Closing " << testApps[step/2] << " : "
                              << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
            QtopiaIpcEnvelope closeApp( QString("QPE/Application/") + testApps[step/2], "close()" );
        }
    }

    // Setup timer to bring us back again
    QTimer::singleShot( stepTime, this, SLOT(runPerformanceTest()) );
    step++;
*/
}

QTOPIA_TASK(QAPerformanceTest, QAPerformanceTest);
