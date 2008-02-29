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

#include "desktop.h"
#include "taskbar.h"
#include "stabmon.h"
#include "launcher.h"

#include <qpe/qpeapplication.h>
#include <qpe/network.h>
#include <qpe/config.h>
#ifdef QT_QWS_CUSTOM
#include <qpe/custom.h>
#endif

#include <qfile.h>
#ifdef QWS
#include <qwindowsystem_qws.h>
#include <qpe/qcopenvelope_qws.h>
#endif
#include <qpe/alarmserver.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#if defined(QT_QWS_CASSIOPEIA) || defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
#include "../calibrate/calibrate.h"
#endif

#ifdef QT_QWS_LOGIN
#include "../login/qdmdialogimpl.h"
#endif

#ifdef QT_QWS_CASSIOPEIA
static void ignoreMessage( QtMsgType, const char * )
{
}
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <qdatetime.h>

void initCassiopeia()
{
    // MIPSEL-specific init - make sure /proc exists for shm
/*
    if ( mount("/dev/ram0", "/", "ext2", MS_REMOUNT | MS_MGC_VAL, 0 ) ) {
	perror("Remounting - / read/write");
    }
*/
    if ( mount("none", "/tmp", "ramfs", 0, 0 ) ) {
	perror("mounting ramfs /tmp");
    } else {
	fprintf( stderr, "mounted /tmp\n" );
    }
    if ( mount("none", "/home", "ramfs", 0, 0 ) ) {
	perror("mounting ramfs /home");
    } else {
	fprintf( stderr, "mounted /home\n" );
    }
    if ( mount("none","/proc","proc",0,0) ) {
	perror("Mounting - /proc");
    } else {
	fprintf( stderr, "mounted /proc\n" );
    }
    if ( mount("none","/mnt","shm",0,0) ) {
	perror("Mounting - shm");
    }
    setenv( "QTDIR", "/", 1 );
    setenv( "QPEDIR", "/", 1 );
    setenv( "HOME", "/home", 1 );
    mkdir( "/home/Documents", 0755 );

    // set a reasonable starting date
    QDateTime dt( QDate( 2001, 3, 15 ) );
    QDateTime now = QDateTime::currentDateTime();
    int change = now.secsTo( dt );

    time_t t = ::time(0);
    t += change;
    stime(&t);

    qInstallMsgHandler(ignoreMessage);
}
#endif

#ifdef QPE_OWNAPM
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <qpe/global.h>

static void disableAPM() 
{

    int fd, cur_val, ret;
    char *device = "/dev/apm_bios";

    fd = open (device, O_WRONLY);

    if (fd ==  -1) {
      perror(device);
      return;
    }

    cur_val = ioctl(fd, APM_IOCGEVTSRC, 0);
    if (cur_val == -1) {
      perror("ioctl");
      exit(errno);
    }

    ret = ioctl(fd, APM_IOCSEVTSRC, cur_val & ~APM_EVT_POWER_BUTTON);
    if (ret == -1) {
        perror("ioctl");
        return;
    }
    close(fd);
}

static void initAPM()
{
    // So that we have to do it ourself, but better.
    disableAPM();
}
#endif

#ifdef QT_DEMO_SINGLE_FLOPPY
#include <sys/mount.h>

void initFloppy()
{
    mount("none","/proc","proc",0,0);
    setenv( "QTDIR", "/", 0 );
    setenv( "HOME", "/root", 0 );
    setenv( "QWS_SIZE", "240x320", 0 );
}
#endif


void initEnvironment()
{
  Config config("locale");
  config.setGroup( "Location" );
  QString tz = config.readEntry( "Timezone", getenv("TZ") );

  // if not timezone set, pick New York 
  if (tz.isNull())
      tz = "America/New_York";

  setenv( "TZ", tz, 1 );
  config.writeEntry( "Timezone", tz);

  config.setGroup( "Language" );
  QString lang = config.readEntry( "Language", getenv("LANG") );
  if ( !lang.isNull() )
    setenv( "LANG", lang, 1 );

  config = Config("qpe");
  config.setGroup( "Rotation" );
  QString dispRep = config.readEntry( "Screen", getenv("QWS_DISPLAY") );

  // if not timezone set, pick New York 
  if (!dispRep.isNull()) {
      setenv( "QWS_DISPLAY", dispRep, 1 );
      config.writeEntry( "Screen", dispRep);
  }

  QString keyOffset = config.readEntry( "Cursor", getenv("QWS_CURSOR_ROTATION") );

  // if not timezone set, pick New York 
  if (keyOffset.isNull())
      keyOffset = "0";

  setenv( "QWS_CURSOR_ROTATION", keyOffset, 1 );
  config.writeEntry( "Cursor", keyOffset);
}

static void initBacklight()
{
#ifndef QT_NO_COP
    QCopEnvelope e("QPE/System", "setBacklight(int)" );
    e << -3; // Forced on
#endif
}



int initApplication( int argc, char ** argv )
{
#ifdef QT_QWS_CASSIOPEIA
    initCassiopeia();
#endif

#ifdef QPE_OWNAPM
    initAPM();
#endif

#ifdef QT_DEMO_SINGLE_FLOPPY
    initFloppy();
#endif

    initEnvironment();

#if !defined(QT_QWS_CASSIOPEIA) && !defined(QT_QWS_IPAQ) && !defined(QT_QWS_EBX)
    setenv( "QWS_SIZE", "240x320", 0 );
#endif

    //Don't flicker at startup:
#ifdef QWS
    QWSServer::setDesktopBackground( QImage() );
#endif
    DesktopApplication a( argc, argv, QApplication::GuiServer );

    initBacklight();

    AlarmServer::initialize();

#if defined(QT_QWS_LOGIN)
    for( int i=0; i<a.argc(); i++ )
      if( strcmp( a.argv()[i], "-login" ) == 0 ) { // No tr
	QDMDialogImpl::login( );
	return 0;
      }
#endif

    Desktop *d = new Desktop();
    a.setMainWidget(d->appLauncher());

    QObject::connect( &a, SIGNAL(launch()),   d, SLOT(raiseLauncher()) );
    QObject::connect( &a, SIGNAL(power()),   d, SLOT(togglePower()) );
    QObject::connect( &a, SIGNAL(backlight()),   d, SLOT(toggleLight()) );
    QObject::connect( &a, SIGNAL(symbol()),   d, SLOT(toggleSymbolInput()) );
    QObject::connect( &a, SIGNAL(numLockStateToggle()),   d, SLOT(toggleNumLockState()) );
    QObject::connect( &a, SIGNAL(capsLockStateToggle()),   d, SLOT(toggleCapsLockState()) );
    QObject::connect( &a, SIGNAL(prepareForRestart()),   d, SLOT(terminateServers()) );
    QObject::connect( &a, SIGNAL(timeChanged()),   d, SLOT(pokeTimeMonitors()) );
    QObject::connect( &a, SIGNAL(appMessage(const QCString&, const QByteArray&)), d, SLOT(appMessage(const QCString&, const QByteArray&)));

    (void)new SysFileMonitor(d);
#ifdef QWS
    Network::createServer(d);
#endif

#if defined(QT_QWS_CASSIOPEIA) || defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
    if ( !QFile::exists( "/etc/pointercal" ) ) {
	// Make sure calibration widget starts on top.
	Calibrate *cal = new Calibrate;
	cal->exec();
	delete cal;
    }
#endif

    d->show();

    int rv =  a.exec();

    delete d;

    return rv;
}

int main( int argc, char ** argv )
{
#ifndef SINGLE_APP
    signal( SIGCHLD, SIG_IGN );
#endif

    int retVal = initApplication( argc, argv );

#ifndef SINGLE_APP
    // Kill them. Kill them all.
    setpgid( getpid(), getppid() );
    killpg( getpid(), SIGTERM );
    sleep( 1 );
    killpg( getpid(), SIGKILL );
#endif

    return retVal;
}

