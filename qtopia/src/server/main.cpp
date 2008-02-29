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

#ifndef QTOPIA_INTERNAL_FILEOPERATIONS
#define QTOPIA_INTERNAL_FILEOPERATIONS
#endif

#define QTOPIA_NO_MAIN

#include "server.h"
#include "serverapp.h"
#include "pda/taskbar.h"
#include "stabmon.h"
#include "pda/launcher.h"

#ifdef QTOPIA_PHONE
#define QTOPIA_USE_SPLASHSCREEN
class SplashScreen;
SplashScreen *splash = 0;
#endif
#ifndef Q_OS_WIN32
#define QTOPIA_USE_FIRSTUSE
#include "firstuse.h"
#endif

#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/network.h>
#include <qtopia/config.h>
#include <qtopia/timezone.h>
#include <qtopia/custom.h>
#include <qtopia/global.h>

#include <qfile.h>
#include <qdir.h>
#ifdef QWS
#include <qwindowsystem_qws.h>
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/alarmserver.h>
#include <qtopia/global.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#define QTOPIA_USE_SPLASHPROCESS
#else
#include <process.h>
#endif

#ifdef QTOPIA_USE_SPLASHPROCESS
#include <qpainter.h>
#include <qasyncimageio.h>
#endif

#ifdef QTOPIA_USE_SPLASHPROCESS
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#endif

#ifdef Q_WS_QWS
#include <qkeyboard_qws.h>
#ifdef SINGLE_EXEC
#include <qsoundqss_qws.h>
#endif
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
	fprintf( stderr, "mounted /tmp\n" ); // No tr
    }
    if ( mount("none", "/home", "ramfs", 0, 0 ) ) {
	perror("mounting ramfs /home");
    } else {
	fprintf( stderr, "mounted /home\n" ); // No tr
    }
    if ( mount("none","/proc","proc",0,0) ) {
	perror("Mounting - /proc");
    } else {
	fprintf( stderr, "mounted /proc\n" ); // No tr
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
#include <qtopia/global.h>

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
    // So that we have to do it ourselves, but better.
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

static void cleanup()
{
    QDir dir( Global::tempDir(), "qcop-msg-*" );

    if (!dir.exists())
        return;
    QStringList stale = dir.entryList();
    QStringList::Iterator it;
    for ( it = stale.begin(); it != stale.end(); ++it ) {
	dir.remove( *it );
    }
}

static void refreshTimeZoneConfig()
{
   // We need to help WorldTime in setting up its configuration for
   //   the current translation
    // BEGIN no tr
    const char *defaultTz[] = {
	"America/New_York",
	"America/Los_Angeles",
	"Europe/Oslo",
	"Asia/Tokyo",
	"Asia/Hong_Kong",
	"Australia/Brisbane",
	0
    };
    // END no tr

    TimeZone curZone;
    QString zoneID;
    int zoneIndex;
    Config cfg = Config( "WorldTime" );
    cfg.setGroup( "TimeZones" );
    if (!cfg.hasKey( "Zone0" )){
	// We have no existing timezones - use the defaults that are untranslated strings
	QString currTz = TimeZone::current().id();
	QStringList zoneDefaults;
	zoneDefaults.append( currTz );
	for ( int i = 0; defaultTz[i] && zoneDefaults.count() < 6; i++ ) {
	    if ( defaultTz[i] != currTz )
		zoneDefaults.append( defaultTz[i] );
	}
	zoneIndex = 0;
	for (QStringList::Iterator it = zoneDefaults.begin(); it != zoneDefaults.end() ; ++it){
	    cfg.writeEntry( "Zone" + QString::number( zoneIndex ) , *it);
	    zoneIndex++;
	}
    }
    // We have an existing list of timezones - refresh the
    //  translations of TimeZone name
    zoneIndex = 0;
    while (cfg.hasKey( "Zone"+ QString::number( zoneIndex ))){
	zoneID = cfg.readEntry( "Zone" + QString::number( zoneIndex ));
	curZone = TimeZone( zoneID );
	if ( !curZone.isValid() ){
	    qDebug( "initEnvironment() Invalid TimeZone %s", zoneID.latin1() );
	    break;
	}
	cfg.writeEntry( "ZoneName" + QString::number( zoneIndex ), curZone.city() );
	zoneIndex++;
    }

}

void initEnvironment(int argc, char ** argv)
{
#ifdef Q_OS_WIN32
    // Config file requires HOME dir which uses QDir which needs the winver
    qt_init_winver();
#endif
    Config config("locale");
    config.setGroup( "Location" );
    QString tz = config.readEntry( "Timezone", getenv("TZ") ).stripWhiteSpace();

    setenv( "TZ", tz, 1 );

    config.setGroup( "Language" );
    QString lang = config.readEntry( "Language", getenv("LANG") ).stripWhiteSpace();
    if( lang.isNull() || lang.isEmpty())
	lang = "en_US";

    setenv( "LANG", lang+".utf8", 1 );

    QString displayID; //get id passed via qpe -display
    
    for (int i = 1; i<argc; i++) {
        QCString arg = argv[i];
        if (arg == "-display") {
            if (++i < argc) {
                QCString id = argv[i];

                QRegExp regExp( ":[0-9]+" );  
                int length;
                int match = regExp.match( id , 0, &length );
                if ( match >= 0 ) {
                    displayID = QString(id).mid( match+1, length-1 );
                    break;
                }
            }
        }
    }
    
    QString qws_display_env(getenv("QWS_DISPLAY"));
    if (!displayID.isEmpty() ) {
        if (!qws_display_env.isNull() && !qws_display_env.isEmpty()) {
            QRegExp regExp( ":[0-9]+" );  
            int length;
            int match = regExp.match( qws_display_env, 0, &length );
            if (match >= 0)
                qws_display_env = qws_display_env.left(match+1)
                        + displayID;
        } else{ 
            qws_display_env = ":" + displayID ;
        }
    }

    if (qws_display_env.isNull() || qws_display_env.isEmpty()) {
        config = Config("qpe");
        config.setGroup( "Rotation" );
        qws_display_env = config.readEntry( "Screen" ).stripWhiteSpace();
    }
    
    if (!qws_display_env.isNull() && !qws_display_env.isEmpty()) {
	setenv( "QWS_DISPLAY", qws_display_env, 1 );
	config.writeEntry( "Screen", qws_display_env);
    }

    QString keyOffset = config.readEntry( "Cursor", getenv("QWS_CURSOR_ROTATION") );

    if (keyOffset.isNull())
	keyOffset = "0";

    setenv( "QWS_CURSOR_ROTATION", keyOffset, 1 );
    config.writeEntry( "Cursor", keyOffset);

    config.write();
}

static void initKeyboard()
{

    Config config("qpe");

    config.setGroup( "Keyboard" );

    int ard = config.readNumEntry( "RepeatDelay" );
    int arp = config.readNumEntry( "RepeatPeriod" );
    if ( ard > 0 && arp > 0 )
	qwsSetKeyboardAutoRepeat( ard, arp );

    QString layout = config.readEntry( "Layout", "us101" );
    Server::setKeyboardLayout( layout );
}

#ifdef QTOPIA_USE_SPLASHPROCESS
class DirectMovie : public QImageConsumer {
public:
    DirectMovie(QWidget* w) : loop(0), period(0)
    {
	size = w->size();
	painter = new QPainter(w);
    }

    virtual ~DirectMovie()
    {
	delete painter;
    }

    static int killed;
    static void splashsig(int)
    {
	killed = 1;
    }

    void play(const QString& filename)
    {
	signal(SIGTERM,splashsig); // I don't want to die... yet.

	const int buf_len=2048;
	uchar buffer[buf_len];
	loop = -1;
	woffset.rx() = -1;

	QFile f(filename);
	if ( f.open(IO_ReadOnly) ) {
	    decoder = new QImageDecoder(this);

	    gettimeofday(&last,0);

	    for (;;) {
		int length = f.readBlock((char*)buffer, buf_len);
		if ( length <= 0 ) {
		    if ( killed || loop < 0 || loop && --loop == 0 )
			break;
		    signal(SIGTERM,SIG_DFL); // Kill me now, I'm looping anyway.
		    length = 0;
		    f.reset();
		    delete decoder;
		    decoder = new QImageDecoder(this);
		}
		uchar* b = buffer;
		int r = -1;
		while (length > 0) {
		    r = decoder->decode(b, length);
		    if ( r <= 0 ) break;
		    b += r;
		    length -= r;
		}
	    }
	}
    }

    void changed(const QRect& r)
    {
	charea |= r;
	//draw(r.topLeft(),r);
    }

    void end() { }

    void preFrameDone()
    {
	if ( woffset.x() == -1 )
	    setSize(decoder->image().width(),decoder->image().height());
    }

    void frameDone(const QPoint& offset, const QRect& r)
    {
	preFrameDone();
	draw(offset,r);
	frameDone();
    }

    void frameDone()
    {
	preFrameDone();
	if ( toobig ) {
	    // Too big; just scale this first frame and exit
	    painter->drawImage(0,0,decoder->image().smoothScale(size.width(),size.height()));
	    exit(0);
	}
	if ( !charea.isEmpty() )
	    draw(charea.topLeft(),charea);
	struct timeval now;
	gettimeofday(&now,0);
	int delay = period*1000 -
	    ((now.tv_sec-last.tv_sec)*1000000+(now.tv_usec-last.tv_usec));
	last.tv_sec += period/1000;
	last.tv_usec += (period%1000)*1000;
	if ( delay > 0 )
	    usleep(delay);
    }

    void setLooping(int l) { loop = l; }
    void setFramePeriod(int p) { period = p; }
    void setSize(int w, int h)
    {
	needfill = size != QSize(w,h);
	toobig = size.width() < w || size.height() < h;
	woffset = (QPoint(size.width(),size.height())-QPoint(w,h))/2;
    }

    void draw(const QPoint& offset, const QRect& r)
    {
	if ( toobig )
	    return;

	if ( needfill ) {
	    painter->fillRect(0,0,size.width(),size.height(),
		QColor(decoder->image().pixel(0,0)));
	    needfill = FALSE;
	}
	painter->drawImage(woffset+offset,decoder->image(),r);
    }


private:
    QRect charea;
    QImageDecoder *decoder;
    QSize size;
    QPoint woffset;
    int loop,period;
    bool needfill,toobig;
    QPainter *painter;
    struct timeval last;
};

int DirectMovie::killed=0;
#endif

#if defined(QTOPIA_USE_SPLASHSCREEN)
class SplashScreen : public QLabel {
    QWidget* rep;
    QImage img;
public:
    SplashScreen() :
	QLabel(0,0),
	rep(0)
    {
    }

    ~SplashScreen()
    {
#ifdef QTOPIA_USE_SPLASHPROCESS
	kill(childpid,SIGTERM);
	waitpid(childpid,0,0);
#endif
    }

    void splash(const QString& imagefile)
    {
#ifdef QTOPIA_USE_SPLASHPROCESS
	setBackgroundMode(NoBackground);
#else
	img = QImage(imagefile);
	QSize ss = qApp->desktop()->rect().size();
	if ( img.size() != ss ) {
	    qWarning("%s != screen size",imagefile.latin1());
	    img = img.smoothScale(ss.width(),ss.height());
	}
#endif
	startTimer(3000); // minimum splash time
	showFullScreen();
	qApp->processEvents();
#ifdef QTOPIA_USE_SPLASHPROCESS
	if ( !(childpid = fork()) ) {
	    DirectMovie(this).play(imagefile);
	    exit(0);
	}
#endif
    }

    void paintEvent(QPaintEvent*)
    {
	QPainter p(this);
	p.drawImage(0,0,img);
    }

    void setReplacement(QWidget *s)
    {
	rep = s;
    }

    void timerEvent(QTimerEvent*)
    {
	killTimers();
	if ( rep ) {
	    rep->show();
	    hide();
	    rep->lower();
	    delete this;
	} else {
	    startTimer(200); // keep waiting
	}
    }
private:
#ifdef QTOPIA_USE_SPLASHPROCESS
    int childpid;
#endif
};
#endif

#ifdef QTOPIA_USE_FIRSTUSE
static bool firstUse()
{
    bool needFirstUse = FALSE;

    {
	Config config( "qpe" );
	config.setGroup( "Startup" );
	needFirstUse |= config.readBoolEntry( "FirstUse", TRUE );
    }

#if defined(QPE_NEED_CALIBRATION)
    if ( !QFile::exists( "/etc/pointercal" ) )
    	needFirstUse = TRUE;
#endif
    if ( !needFirstUse )
	return FALSE;
    qWarning("Need to run firstuse");

    ServerApplication::login(TRUE);

    FirstUse *fu = new FirstUse();
#ifdef QTOPIA_USE_SPLASHSCREEN
    delete splash;
#endif
    fu->exec();
    bool rs = fu->restartNeeded();
    qWarning("firstuse finished %d",rs);
    delete fu;
    return rs;
}
#endif


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

    initEnvironment(argc, argv);

    cleanup();

    //Don't flicker at startup:
#ifdef QWS
    QWSServer::setDesktopBackground( QImage() );
#endif
    ServerApplication a( argc, argv, QApplication::GuiServer );


    refreshTimeZoneConfig();

    initKeyboard();

#if defined(QTOPIA_USE_SPLASHSCREEN) && defined(QTOPIA_PHONE)
    splash = new SplashScreen;
    splash->splash(Resource::findPixmap("splash"));
#endif
#ifdef QTOPIA_USE_FIRSTUSE
    if ( Global::mousePreferred() && firstUse() ) {
	a.restart();
	return 0;
    }
#endif

    Server *s = new Server();
    s->connect(&a, SIGNAL(recoverMemory(ServerApplication::MemState)),
		SLOT(recoverMemory(ServerApplication::MemState)));

    ServerApplication::login(TRUE);

#ifndef QPE_SYSTEM_SYSFILEMONITOR
    // Device *should* send QCop message when a change happens.
    (void)new SysFileMonitor(s);
#endif

#ifdef QWS
    Network::createServer(s);
#endif

#if defined(QTOPIA_USE_SPLASHSCREEN)
    splash->setReplacement(s);
#else
    s->show();
#endif

    AlarmServer::initialize();

    int rv =  a.exec();

    qDebug("exiting...");
    delete s;

    return rv;
}

#ifndef Q_OS_WIN32

#ifdef SINGLE_EXEC
#include "../tools/qcop/qcopimpl.h"
#include "singleexec_quicklaunch.h"
#include "singleexec_server_apps.cpp"
#include <qtopia/resource.h>

int main( int argc, char ** argv )
{
    QString executableName(argv[0]); 
    executableName = executableName.right(executableName.length() 
	    - executableName.findRev('/') - 1); 

    if ( executableName != "qpe" ) {
	if ( executableName == "quicklauncher" ) {
	    app = new QPEApplication( argc, argv ); 
#ifdef _OS_LINUX_
	    // Setup to change proc title
	    int i;
	    char **envp = environ;
	    /* Move the environment so we can reuse the memory.
	     * (Code borrowed from sendmail.) */
	    for (i = 0; envp[i] != NULL; i++)
		continue;
	    environ = (char **) malloc(sizeof(char *) * (i + 1));
	    if (environ == NULL)
		return -1;
	    for (i = 0; envp[i] != NULL; i++)
		if ((environ[i] = strdup(envp[i])) == NULL)
		    return -1;
	    environ[i] = NULL;

	    argv0 = argv;
	    if (i > 0)
		argv_lth = envp[i-1] + strlen(envp[i-1]) - argv0[0];
	    else
		argv_lth = argv0[argc-1] + strlen(argv0[argc-1]) - argv0[0];
#endif
	    unsetenv( "LD_BIND_NOW" );
	    (void)new QuickLauncher();
	    qDebug( "QuickLauncher running" );
	    // Pre-load default fonts
	    QFontMetrics fm( QApplication::font() );
	    fm.ascent(); // causes font load.
	    QFont f( QApplication::font() );
	    f.setWeight( QFont::Bold );
	    QFontMetrics fmb( f );
	    fmb.ascent(); // causes font load.

	    // Create a widget to force initialization of title bar images, etc.
	    QObject::disconnect(app, SIGNAL(lastWindowClosed()), app, SLOT(hideOrQuit()));
	    QWidget *w = new QWidget(0,0,Qt::WDestructiveClose|Qt::WStyle_ContextHelp|Qt::WStyle_Tool);
	    w->setGeometry( -100, -100, 10, 10 );
	    w->show();
	    QTimer::singleShot( 0, w, SLOT(close()) );

	    // Each of the following force internal structures/internal
	    // initialization to be performed.  This may mean allocating
	    // memory that is not needed by all applications.
	    TimeZone::current().isValid(); // populate timezone cache
	    TimeString::currentDateFormat(); // create internal structures
	    TimeString::currentAMPM();
	    Resource::loadIconSet("new"); // do internal init
	    app->enter_loop();
	} else if ( executableName == "qcop" ) {
	    return doqcopimpl(argc,argv);
	} else if ( executableName == "qss" ) {
	    QApplication a(argc,argv, false);
	    (void)new QWSSoundServer(0);
	    setpriority(PRIO_PROCESS, 0, -15);
	    return a.exec();
	} else {
	    QuickLauncher::exec(argc,argv);
	}
	int rv = app->exec();
	delete mainWindow;
	delete app;
	return rv;
    }

#else // SINGLE_EXEC
int main( int argc, char ** argv )
{
#endif // SINGLE_EXEC

    signal( SIGCHLD, SIG_IGN );

    int retVal = initApplication( argc, argv );

#ifdef Q_WS_QWS
    // Have we been asked to restart?
    if ( ServerApplication::doRestart ) {
	for ( int fd = 3; fd < 100; fd++ )
	    close( fd );
# if defined(QT_DEMO_SINGLE_FLOPPY)
	execl( "/sbin/init", "qpe", 0 );
# elif defined(QT_QWS_CASSIOPEIA)
	execl( "/bin/sh", "sh", 0 );
# else
	execl( (QPEApplication::qpeDir()+"bin/qpe").latin1(), "qpe", 0 );
# endif
    }
#endif

    // Kill them. Kill them all.
    setpgid( getpid(), getppid() );
    killpg( getpid(), SIGTERM );
    Global::sleep( 1 );
    killpg( getpid(), SIGKILL );

    return retVal;
}
#else // WIN32

int main( int argc, char ** argv )
{
    int retVal = initApplication( argc, argv );

    if ( DesktopApplication::doRestart ) {
	qDebug("Trying to restart");
	execl( (QPEApplication::qpeDir()+"bin\\qpe").latin1(), "qpe", 0 );
    }

    return retVal;
}

#endif // WIN32
