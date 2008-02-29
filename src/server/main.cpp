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

#include "pda/taskbar.h"
#include "stabmon.h"
#include "qabstractserverinterface.h"

#ifdef QTOPIA_PDA
#include "pda/launcher.h"
#include "pda/firstuse.h"
#endif

#include <qtopiaapplication.h>
#include <qtimezone.h>
#include <custom.h>

#include <qfile.h>
#include <qdir.h>
#include <qimagereader.h>
#include <qscreen_qws.h>
#include <qsettings.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include <qtopiaipcenvelope.h>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <qbootsourceaccessory.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#else
#include <process.h>
#endif

#include <QPainter>
#include <QThread>
#include <QBasicTimer>

#include <QImageIOHandler>

#include <perftest.h>

QSXE_APP_KEY

#include "qtopiaserverapplication.h"

#include <QDesktopWidget>
#include <QLibraryInfo>

#include <sys/types.h>
#include <unistd.h>

#include <QValueSpaceItem>
#include <QKeyEvent>
#include <ThemedView>

#ifdef QPE_OWNAPM
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/ioctl.h>

// Not currently supported, available for demonstration purposes.
//#define QTOPIA_ANIMATED_SPLASH

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

    QTimeZone curZone;
    QString zoneID;
    int zoneIndex;
    QSettings cfg( "WorldTime" );
    cfg.beginGroup( "TimeZones" );
    if (!cfg.contains( "Zone0" )){
        // We have no existing timezones - use the defaults that are untranslated strings
        QString currTz = QTimeZone::current().id();
        QStringList zoneDefaults;
        zoneDefaults.append( currTz );
        for ( int i = 0; defaultTz[i] && zoneDefaults.count() < 6; i++ ) {
            if ( defaultTz[i] != currTz )
                zoneDefaults.append( defaultTz[i] );
        }
        zoneIndex = 0;
        for (QStringList::Iterator it = zoneDefaults.begin(); it != zoneDefaults.end() ; ++it){
            cfg.setValue( "Zone" + QString::number( zoneIndex ) , *it);
            zoneIndex++;
        }
    }
    // We have an existing list of timezones - refresh the
    //  translations of QTimeZone name
    zoneIndex = 0;
    while (cfg.contains( "Zone"+ QString::number( zoneIndex ))){
        zoneID = cfg.value( "Zone" + QString::number( zoneIndex )).toString();
        curZone = QTimeZone( zoneID.toLatin1() );
        if ( !curZone.isValid() ){
            qWarning( "initEnvironment() Invalid QTimeZone %s", (const char *)zoneID.toLatin1() );
            break;
        }
        zoneIndex++;
    }

}

static void initKeyboard()
{
    QSettings config("Trolltech","qpe");

    config.beginGroup( "Keyboard" );

/* FIXME
    int ard = config.value( "RepeatDelay" ).toInt();
    int arp = config.value( "RepeatPeriod" ).toInt();
    if ( ard > 0 && arp > 0 )
        qwsSetKeyboardAutoRepeat( ard, arp );
*/

    QString layout = config.value( "Layout", "us101" ).toString();
}

#ifdef QTOPIA_PDA
static bool firstUse()
{
    bool needFirstUse = false;
#if 0 //  defined(QPE_NEED_CALIBRATION)
    if ( !QFile::exists( "/etc/pointercal" ) )
        needFirstUse = true;
#endif

    {
        QSettings config("Trolltech","qpe");
        config.beginGroup( "Startup" );
        needFirstUse |= config.value( "FirstUse", true ).toBool();
    }

    if ( !needFirstUse )
        return false;

    ServerApplication::login(true);

    FirstUse *fu = new FirstUse();
    fu->exec();
    bool rs = fu->restartNeeded();
    delete fu;
    return rs;
}
#endif

#if defined(QTOPIA_ANIMATED_SPLASH)
class DirectMovie : public QObject, public QImageReader {
public:
    DirectMovie() {}

    void play(const QString& filename)
    {
        setFileName(filename);

        if ( !canRead() )
            return;

        gettimeofday(&last,0);
        timer.start(0,this);
    }

    void timerEvent(QTimerEvent*)
    {
        QImage img = read();
        int delay = 0;

        if ( img.isNull() ) {
            setFileName(fileName()); // XXX restart
        } else {
            QRegion r(0,0,qt_screen->width(),qt_screen->height());
            qt_screen->blit(img,QPoint(0,0),r);
            qt_screen->exposeRegion(r,1);
            if ( imageCount() == 1 )
                return;
            struct timeval now;
            gettimeofday(&now,0);
            int period = nextImageDelay();
            delay = period -
                ((now.tv_sec-last.tv_sec)*1000+(now.tv_usec-last.tv_usec)/1000);
            last.tv_sec += period/1000;
            last.tv_usec += (period%1000)*1000;
            if ( delay < 0 )
                delay = 0;
        }

        timer.start(delay,this);
    }

private:
    struct timeval last;
    QBasicTimer timer;
};


class DirectMovieThread : public QThread {
public:
    DirectMovieThread(QObject *parent) : QThread(parent)
    {
    }

    void play(const QString& file)
    {
        filename = file;
        start();
    }

    void run()
    {
        movie = new DirectMovie;
        movie->play(filename);
        exec();
        delete movie;
    }

private:
    QString filename;
    DirectMovie *movie;
};


class SplashScreen : public QLabel {
public:
    SplashScreen() :
        QLabel(0),
        rep(0),
        thread(0)
    {
    }

    ~SplashScreen()
    {
        if ( thread ) {
            thread->terminate();
            thread->wait();
            delete thread;
        }
    }

    void splash(const QString& animfile)
    {
        setAttribute(Qt::WA_NoBackground);
        timer.start(2000,this); // minimum splash time

        // Conceptually, we want to paint "on this widget", but instead
        // we paint on the screen, since that can be done in a thread.
        //showFullScreen();

        thread = new DirectMovieThread(this);
        thread->play(animfile);
    }

    void setReplacement(QWidget *s)
    {
        rep = s;
    }

    void timerEvent(QTimerEvent*)
    {
        if ( rep ) {
            rep->show();
            hide();
            rep->lower();
            delete this;
        } else {
            timer.start(200,this); // keep waiting
        }
    }
private:
    QWidget* rep;
    QImage img;
    DirectMovieThread *thread;
    QBasicTimer timer;
};
#endif

class BootCharger : public ThemedView {
    Q_OBJECT
public:
    BootCharger() :
        ThemedView(),
        vsoCharging("/Accessories/Battery/Charging")
    {
        QSettings qpeCfg("Trolltech", "qpe");
        qpeCfg.beginGroup("Appearance");
        QString themeDir = Qtopia::qtopiaDir() + "etc/themes/";
        QString theme = qpeCfg.value("Theme").toString();

        QSettings themeCfg(themeDir + theme, QSettings::IniFormat);
        themeCfg.beginGroup("Theme");
        if (themeCfg.contains("BootChargerConfig")) {
            QString themeFile = themeCfg.value("BootChargerConfig").toString();
        
            setSourceFile(themeDir + themeFile);
            if (loadSource())
                layout();
            else
                qWarning("Invalid BootCharger theme.");
        } else {
            qWarning("Invalid BootCharger theme.");
        }

        connect(&vsoCharging, SIGNAL(contentsChanged()), SLOT(chargingStateChanged()));
        grabKeyboard();
    }

    ~BootCharger()
    {
        releaseKeyboard();
    }

    void keyPressEvent(QKeyEvent *e)
    {
        if (e->key() == Qt::Key_Hangup) {
            emit finished();
        }
    }

    bool charging()
    {
        return vsoCharging.value().toBool();
    }

signals:
    void finished();

public slots:
    void chargingStateChanged()
    {
        if (!vsoCharging.value().toBool()) {
            QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
        }
    }

private:
    QValueSpaceItem vsoCharging;
};

void Qtopia_initAlarmServer(); // from qalarmserver.cpp

//
// IMPORTANT NOTE: This function implements part of the Qtopia startup
// processing.  The sequence of steps performed during Qtopia startup is
// documented in the file <depot-directory>/doc/src/qtopia-startup.qdoc.
// If you make changes or additions to the startup sequence, you must ensure
// that that document is also updated.
//
int initApplication( int argc, char ** argv )
{
    if(!QtopiaServerApplication::startup(argc, argv, QList<QByteArray>() << "prestartup"))
        qFatal("Unable to initialize task subsystem.  Please check '%s' exists and its content is valid.", QtopiaServerApplication::taskConfigFile().toLatin1().constData());

#ifdef QPE_OWNAPM
    initAPM();
#endif

    refreshTimeZoneConfig();
    qLog(Performance) << "QtopiaServer : " << "Refresh time zone information : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );

    initKeyboard();
    qLog(Performance) << "QtopiaServer : " << "Keyboard initialisation : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );

    const QByteArray warmBootFile = "/tmp/warm-boot";
    if ( !QFile::exists( warmBootFile ) &&
         QValueSpaceItem("/Accessories/Battery/Charging").value().toBool() ) {

        QBootSourceAccessory bsa;
        if (bsa.bootSource() == QBootSourceAccessory::Charger) {

            BootCharger *bootCharger = new BootCharger;
            bootCharger->showFullScreen();

            QEventLoop eventloop;
            eventloop.connect(bootCharger, SIGNAL(finished()), SLOT(quit()));
            eventloop.exec();

            delete bootCharger;
        }
    }
    ::system( "touch " + warmBootFile );

#if defined(QTOPIA_ANIMATED_SPLASH)
    SplashScreen *splash = new SplashScreen;
    splash->splash(QString(":image/splash"));
    qLog(Performance) << "QtopiaServer : " << "Splash screen creation : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
#endif

    if(!QtopiaServerApplication::startup(argc, argv, QList<QByteArray>() << "startup"))
        qFatal("Unable to initialize task subsystem.  Please check '%s' exists and its content is valid.", QtopiaServerApplication::taskConfigFile().toLatin1().constData());


#if 0 // FIXME first use is broken. just disable it for now
    // Don't use first use under Windows
# if defined(Q_OS_UNIX)
    if ( firstUse() ) {
        a.restart();
        return 0;
    }
# endif
#endif

    // Load and show UI
    QAbstractServerInterface *interface =
        qtopiaWidget<QAbstractServerInterface>(0, Qt::FramelessWindowHint);

#if defined(QTOPIA_ANIMATED_SPLASH)
    splash->setReplacement(interface);
#else
    if(interface)
        interface->show();
#endif
    qLog(Performance) << "QtopiaServer : " << "Display the server : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );

    Qtopia_initAlarmServer();
    qLog(Performance) << "QtopiaServer : " << "AlarmServer startup : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );

    qLog(Performance) << "QtopiaServer : " << "Entering event loop : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );

//    int rv =  a.exec();
    //Can't do this either, exec is static int rv =  qApp->exec();
    int rv = static_cast<QtopiaApplication *>(qApp)->exec();

    qLog(QtopiaServer) << "exiting...";
    qLog(Performance) << "QtopiaServer : " << "Event loop exited : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );

    delete interface;

    qLog(Performance) << "QtopiaServer : " << "Leaving initApplication : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    return rv;
}

#ifndef Q_OS_WIN32

static void check_prefix()
{
    // Construct a string with the path to the binary that we'll expect to see at runtime
    QString prefix_bin = QDir(QString("%1/bin/%2").arg(QLibraryInfo::location(QLibraryInfo::PrefixPath)).arg(QTOPIA_TARGET)).absolutePath();
    QString prefix_check = QDir(prefix_bin).canonicalPath(); // handle symlinks
    // Get the path to the currently running binary
    QString proc_check = QDir(QString("/proc/%1/exe").arg(getpid())).canonicalPath();
    if ( proc_check.isEmpty() )
        proc_check = QString("CANNOT READ /proc/%1/exe").arg(getpid());
    if ( prefix_check != proc_check ) {
        // Figure out what the prefix should be based on the location of the currently running binary
        QString proc_prefix = QDir(QString("%1/../..").arg(proc_check)).canonicalPath();
        qWarning() << "**********************************************************" << endl
                   << "* ERROR: Expecting this binary to be located in" << endl
                   << "*  " << prefix_bin.toLocal8Bit().constData() << endl
                   << "* but it is being run from" << endl
                   << "*  " << proc_check.toLocal8Bit().constData() << endl
                   << "*" << endl
                   << "* This generally indicates that you have specified the wrong" << endl
                   << "* value for -prefix when configuring Qtopia. Based on the" << endl
                   << "* location of this binary, you should be using a prefix of" << endl
                   << "*  " << proc_prefix.toLocal8Bit().constData() << endl
                   << "**********************************************************" << endl;
    }
}

#ifdef SINGLE_EXEC
#include "../tools/quicklauncher/quicklaunch.h"
int main_quicklaunch( int argc, char **argv );

int main( int argc, char ** argv )
{
    check_prefix();
    QString executableName(argv[0]);
    executableName = executableName.right(executableName.length() - executableName.lastIndexOf('/') - 1);

    if ( executableName != "qpe" ) {
        qLog(Performance) << executableName.toLatin1().constData() << " : " << "Starting single-exec main : "
            << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
        QPEMainMap *qpeMainMap();
        if ( qpeMainMap()->contains(executableName) ) {
            // This is a non-quicklaunch app
            return (*qpeMainMap())[executableName](argc, argv);
        } else {
            // Someone directly invoked a quicklaunch app
            // Pass control to quicklaunch's main function so that it can deal with it
            return main_quicklaunch(argc, argv);
        }
    }

#else // SINGLE_EXEC
int main( int argc, char ** argv )
{
    check_prefix();
#endif // SINGLE_EXEC
    qLog(Performance) << "QtopiaServer : " << "Starting qpe main : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );

    signal( SIGCHLD, SIG_IGN );

    const QByteArray restartFile = "/tmp/restart-qtopia";
    ::system( "touch " + restartFile );

    int retVal = initApplication( argc, argv );

#ifdef Q_WS_QWS
    // Have we been asked to restart?
    if(!(QtopiaServerApplication::shutdownType() == QtopiaServerApplication::RestartDesktop)) {
        //we assume that the calling script is running
        //in a loop which will restart qpe automatically
        //if /tmp/restart-qtopia exists. The script may or
        //may not delete the file
        if ( QFile::exists( restartFile ) )
            QFile::remove( restartFile );
    }
#endif

    // Kill them. Kill them all.
    setpgid( getpid(), getppid() );
    killpg( getpid(), SIGTERM );
    Qtopia::sleep( 1 );
    killpg( getpid(), SIGKILL );

    qLog(Performance) << "QtopiaServer : " << "Exiting qpe main : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    return retVal;
}
#else // WIN32

int main( int argc, char ** argv )
{
    int retVal = initApplication( argc, argv );

    if ( DesktopApplication::doRestart ) {
        qLog(QtopiaServer) << "Trying to restart";
        execl( (Qtopia::qtopiaDir()+"bin\\qpe").toLatin1(), "qpe", 0 );
    }

    return retVal;
}

#endif // WIN32

#include "main.moc"

