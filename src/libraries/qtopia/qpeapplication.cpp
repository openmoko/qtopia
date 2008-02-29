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
#define QTOPIA_INTERNAL_LANGLIST
#include <stdlib.h>
#include <unistd.h>
#include <qfile.h>
#ifdef Q_WS_QWS
#ifndef QT_NO_COP
#if QT_VERSION <= 231
#define private public
#define sendLocally processEvent
#include "qcopenvelope_qws.h"
#undef private
#else
#include "qcopenvelope_qws.h"
#endif
#endif
#include <qwindowsystem_qws.h>
#endif
#include <qtextstream.h>
#include <qpalette.h>
#include <qbuffer.h>
#include <qptrdict.h>
#include <qregexp.h>
#include <qdir.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qdragobject.h>
#include <qtextcodec.h>
#include <qevent.h>
#include <qtooltip.h>
#include <qsignal.h>
#include "qpeapplication.h"
#include "qpestyle.h"
#if QT_VERSION >= 300
#include <qstylefactory.h>
#else
#include <qwindowsstyle.h>
#endif
#include "global.h"
#include "resource.h"
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
#include "qutfcodec.h"
#endif
#include "config.h"
#include "network.h"
#ifdef QWS
#include "fontmanager.h"
#endif
#include "power.h"
#include "alarmserver.h"
#include "applnk.h"
#include "qpemenubar.h"
#include "qlibrary.h"
#include "imagecodecinterface.h"
#include "textcodecinterface.h"
#include "styleinterface.h"
#include "fontdatabase.h"

#include <unistd.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

// for setBacklight()
#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <stdlib.h>


class QPEApplicationData {
public:
    QPEApplicationData() : presstimer(0), presswidget(0), rightpressed(FALSE),
	kbgrabber(0), kbregrab(FALSE), notbusysent(FALSE), preloaded(FALSE),
	forceshow(FALSE), nomaximize(FALSE), qpe_main_widget(0),
	keep_running(TRUE), skiptimechanged(FALSE), styleLib(0), styleIface(0)
    {
	qcopq.setAutoDelete(TRUE);
    }

    int presstimer;
    QWidget* presswidget;
    QPoint presspos;
    bool rightpressed;
    int kbgrabber;
    bool kbregrab;
    bool notbusysent;
    QString appName;
    struct QCopRec {
	QCopRec(const QCString &ch, const QCString &msg,
                               const QByteArray &d) :
	    channel(ch), message(msg), data(d) { }

	QCString channel;
	QCString message;
	QByteArray data;
    };
    bool preloaded;
    bool forceshow;
    bool nomaximize;
    QWidget* qpe_main_widget;
    bool keep_running;
    int skiptimechanged;
    QList<QCopRec> qcopq;

    void enqueueQCop(const QCString &ch, const QCString &msg,
                               const QByteArray &data)
    {
	qcopq.append(new QCopRec(ch,msg,data));
    }
    void sendQCopQ()
    {
	QCopRec* r;
#ifndef QT_NO_COP
	for (QListIterator<QCopRec> it(qcopq); (r=it.current()); ++it)
	    QCopChannel::sendLocally(r->channel,r->message,r->data);
#endif
	qcopq.clear();
    }

    static void show_mx(QWidget* mw, bool nomaximize)
    {
	if ( mw->layout() && mw->inherits("QDialog") ) {
	    QPEApplication::showDialog((QDialog*)mw,nomaximize);
	} else {
	    if ( !mw->isVisible() ) {
		int x = (qApp->desktop()->width()-mw->frameGeometry().width())/2;
		int y = (qApp->desktop()->height()-mw->frameGeometry().height())/2;
		mw->move( QMAX(x,0), QMAX(y,0) );
#ifdef Q_WS_QWS
		if ( !nomaximize )
		    mw->showMaximized();
		else
#endif
		mw->show();
	    } else {
		mw->raise();
	    }
	}
    }

    static bool setWidgetCaptionFromAppName( QWidget* /*mw*/, const QString& /*appName*/, const QString& /*appsPath*/ )
    {
    /*
	// This works but disable it for now until it is safe to apply
	// What is does is scan the .desktop files of all the apps for
	// the applnk that has the corresponding argv[0] as this program
	// then it uses the name stored in the .desktop file as the caption
	// for the main widget. This saves duplicating translations for
	// the app name in the program and in the .desktop files.

	AppLnkSet apps( appsPath );

	QList<AppLnk> appsList = apps.children();
	for ( QListIterator<AppLnk> it(appsList); it.current(); ++it ) {
	    if ( (*it)->exec() == appName ) {
		mw->setCaption( (*it)->name() );
		return TRUE;
	    }
	}
    */
	return FALSE;
    }


    void show(QWidget* mw, bool nomax)
    {
	setWidgetCaptionFromAppName( mw, appName, QPEApplication::qpeDir() + "apps" );
	nomaximize = nomax;
	qpe_main_widget = mw;
#ifndef QT_NO_COP
	sendQCopQ();
#endif
	if ( preloaded ) {
	    if(forceshow)
		show_mx(mw,nomax);
	} else if ( keep_running ) {
	    show_mx(mw,nomax);
	}
    }

    void loadTextCodecs()
    {
	QString path = QPEApplication::qpeDir() + "/plugins/textcodecs";
	QDir dir( path, "lib*.so" );
	QStringList list = dir.entryList();
	QStringList::Iterator it;
	for ( it = list.begin(); it != list.end(); ++it ) {
	    TextCodecInterface *iface = 0;
	    QLibrary *lib = new QLibrary( path + "/" + *it );
	    if ( lib->queryInterface( IID_QtopiaTextCodec, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
		QValueList<int> mibs = iface->mibEnums();
		for (QValueList<int>::ConstIterator i=mibs.begin(); i!=mibs.end(); ++i) {
		    (void)iface->createForMib(*i);
		    // ### it exists now; need to remember if we can delete it
		}
	    } else {
		lib->unload();
		delete lib;
	    }
	}
    }

    void loadImageCodecs()
    {
	QString path = QPEApplication::qpeDir() + "/plugins/imagecodecs";
	QDir dir( path, "lib*.so" );
	QStringList list = dir.entryList();
	QStringList::Iterator it;
	for ( it = list.begin(); it != list.end(); ++it ) {
	    ImageCodecInterface *iface = 0;
	    QLibrary *lib = new QLibrary( path + "/" + *it );
	    if ( lib->queryInterface( IID_QtopiaImageCodec, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
		QStringList formats = iface->keys();
		for (QStringList::ConstIterator i=formats.begin(); i!=formats.end(); ++i) {
		    (void)iface->installIOHandler(*i);
		    // ### it exists now; need to remember if we can delete it
		}
	    } else {
		lib->unload();
		delete lib;
	    }
	}
    }

    QLibrary *styleLib;
    StyleInterface *styleIface;
    QString styleName;
    QString decorationName;
};


class ResourceMimeFactory : public QMimeSourceFactory {
public:
    ResourceMimeFactory()
    {
	setFilePath( Global::helpPath() );
	setExtensionType("html","text/html;charset=UTF-8");
    }

    const QMimeSource* data(const QString& abs_name) const
    {
	const QMimeSource* r = QMimeSourceFactory::data(abs_name);
	if ( !r ) {
	    int sl = abs_name.length();
	    do {
		sl = abs_name.findRev('/',sl-1);
		QString name = sl>=0 ? abs_name.mid(sl+1) : abs_name;
		int dot = name.findRev('.');
		if ( dot >= 0 )
		    name = name.left(dot);
		QImage img = Resource::loadImage(name);
		if ( !img.isNull() )
		    r = new QImageDrag(img);
	    } while (!r && sl>0);
	}
	return r;
    }
};

static int muted=0;

static void setVolume(int t=0, int percent=-1)
{
    switch (t) {
	case 0: {
	    Config cfg("Sound");
	    cfg.setGroup("System");
	    if ( percent < 0 )
		percent = cfg.readNumEntry("Volume",50);
	    int fd = 0;
	    if ((fd = open("/dev/mixer", O_RDWR))>=0) {
		int vol = muted ? 0 : percent;
		// set both channels to same volume
		vol |= vol << 8;
		ioctl(fd, MIXER_WRITE(0), &vol);
		::close(fd);
	    }
	} break;
    }
}

int qpe_sysBrightnessSteps()
{
#if defined(QT_QWS_IPAQ)
    return 255;
#elif defined(QT_QWS_EBX)
    return 4;
#else
    return 255; // ?
#endif
}


static int& hack(int& i)
{
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
    // These should be created, but aren't in Qt 2.3.0
    (void)new QUtf8Codec;
    (void)new QUtf16Codec;
#endif
    return i;
}

static bool forced_off = FALSE;
static int curbl=-1;

static int backlight()
{
    if ( curbl == -1 ) {
	// Read from config
	Config config( "qpe" );
	config.setGroup( "Screensaver" );
	curbl = config.readNumEntry("Brightness",255);
    }
    return curbl;
}

static void setBacklight(int bright)
{
    if ( bright == -3 ) {
	// Forced on
	forced_off = FALSE;
	bright = -1;
    }
    if ( forced_off && bright != -2 )
	return;
    if ( bright == -2 ) {
	// Toggle between off and on
	bright = curbl ? 0 : -1;
	forced_off = !bright;
    }
    if ( bright == -1 ) {
	// Read from config
	Config config( "qpe" );
	config.setGroup( "Screensaver" );
	bright = config.readNumEntry("Brightness",255);
    }
#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
    if ( QFile::exists("/usr/bin/bl") ) {
	QString cmd = "/usr/bin/bl 1 ";
	cmd += bright<=0 ? "0 " : "1 ";
	cmd += QString::number(bright);
	system(cmd.latin1());
#if defined(QT_QWS_EBX)
    } else if ( QFile::exists("/dev/fl") ) {
#define FL_IOCTL_STEP_CONTRAST    100
	int fd = open("/dev/fl", O_WRONLY);
	if (fd >= 0 ) {
	    int steps = qpe_sysBrightnessSteps();
	    int bl = ( bright * steps + 127 ) / 255;
	    if ( bright && !bl ) bl = 1;
	    bl = ioctl(fd, FL_IOCTL_STEP_CONTRAST, bl);
	    close(fd);
	}
    }
#elif defined(QT_QWS_IPAQ)
    } else if ( QFile::exists("/dev/ts") || QFile::exists("/dev/h3600_ts") ) {
	typedef struct {
	    unsigned char mode;
	    unsigned char pwr;
	    unsigned char brightness;
	} FLITE_IN;
# ifndef FLITE_ON
#  ifndef _LINUX_IOCTL_H
#   include <linux/ioctl.h>
#  endif
#  define FLITE_ON                _IOW('f', 7, FLITE_IN)
# endif
	int fd;
	if ( QFile::exists("/dev/ts") )
	    fd = open("/dev/ts", O_WRONLY);
	else
	    fd = open("/dev/h3600_ts", O_WRONLY);
	if (fd >= 0 ) {
	    FLITE_IN bl;
	    bl.mode = 1;
	    bl.pwr = bright ? 1 : 0;
	    bl.brightness = bright;
	    ioctl(fd, FLITE_ON, &bl);
	    close(fd);
	}
    }
#endif
#endif
    curbl = bright;
}

void qpe_setBacklight(int bright) { setBacklight(bright); }

static bool dim_on = FALSE;
static bool lightoff_on = FALSE;
static bool networkedsuspend_on = FALSE;
static bool poweredsuspend_on = FALSE;
static int disable_suspend = 100;

static bool powerOnlineStopsSuspend()
{
    return !poweredsuspend_on && PowerStatusManager::readStatus().acStatus() == PowerStatus::Online;
}

static bool networkOnlineStopsSuspend()
{
#ifdef QWS
    return !networkedsuspend_on && Network::networkOnline();
#endif
}

#ifdef QWS
class QPEScreenSaver : public QWSScreenSaver
{

public:
    QPEScreenSaver()
    {
    }
    void restore()
    {
	setBacklight(-1);
    }
    bool save(int level)
    {
	switch ( level ) {
	 case 0:
	    if ( disable_suspend > 0 && dim_on ) {
		if (backlight() > 1)
		    setBacklight(1); // lowest non-off
	    }
	    return TRUE;
	    break;
	 case 1:
	    if ( disable_suspend > 1 && lightoff_on ) {
		setBacklight(0); // off
	    }
	    return TRUE;
	    break;
	 case 2:
	    if ( disable_suspend > 2 && !powerOnlineStopsSuspend() && !networkOnlineStopsSuspend() )
	    {
		QWSServer::processKeyEvent( 0xffff, Qt::Key_F34, FALSE, TRUE, FALSE );
		return FALSE;
	    }
	    return TRUE;
	    break;
	}
	return FALSE;
    }
};
#endif

static int ssi(int interval, Config &config, const QString &enable, const QString& value, int def)
{
    if ( !enable.isEmpty() && config.readNumEntry(enable,0) == 0 )
	return 0;

    if ( interval < 0 ) {
	// Restore screen blanking and power saving state
	interval = config.readNumEntry( value, def );
    }
    return interval;
}

static void setScreenSaverIntervals(int i1, int i2, int i3)
{
    Config config( "qpe" );
    config.setGroup( "Screensaver" );

    int v[4];
    i1 = ssi(i1, config, "Dim","Interval_Dim", 30);
    i2 = ssi(i2, config, "LightOff","Interval_LightOff", 20);
    i3 = ssi(i3, config, "","Interval", 60);

    //qDebug("screen saver intervals: %d %d %d", i1, i2, i3);

    v[0] = QMAX( 1000*i1, 100);
    v[1] = QMAX( 1000*i2, 100);
    v[2] = QMAX( 1000*i3, 100);
    v[3] = 0;
    dim_on = ( (i1 != 0) ? config.readNumEntry("Dim",1) : FALSE );
    lightoff_on = ( (i2 != 0 ) ? config.readNumEntry("LightOff",1) : FALSE );
    poweredsuspend_on = config.readNumEntry("Suspend",0);
    networkedsuspend_on = config.readNumEntry("NetworkedSuspend",1);
#ifdef QWS
    if ( !i1 && !i2 && !i3 )
	QWSServer::setScreenSaverInterval(0);
    else
	QWSServer::setScreenSaverIntervals(v);
#endif
}

static void setScreenSaverInterval(int interval)
{
    setScreenSaverIntervals(-1,-1,interval);
}


/*!
  \class QPEApplication qpeapplication.h
  \brief The QPEApplication class implements various system services
    that are available to all Qtopia applications.

  Simply by using QPEApplication instead of QApplication, a standard Qt
  application becomes a Qtopia application. It automatically follows
  style changes, quits and raises, and in the
  case of \link docwidget.html document-oriented\endlink applications,
  changes the currently displayed document in response to the environment.

  To create a \link docwidget.html document-oriented\endlink
  application use showMainDocumentWidget(); to create a
  non-document-oriented application use showMainWidget(). The
  keepRunning() function indicates whether the application will
  continue running after it's processed the last \link qcop.html
  QCop\endlink message. This can be changed using setKeepRunning().

  A variety of signals are emitted when certain events occur, for
  example, timeChanged(), clockChanged(), weekChanged(),
  dateFormatChanged() and volumeChanged(). If the application receives
  a \link qcop.html QCop\endlink message on the application's
  QPE/Application/\e{appname} channel, the appMessage() signal is
  emitted. There are also flush() and reload() signals, which
  are emitted when synching begins and ends respectively - upon these
  signals, the application should save and reload any data
  files that are involved in synching. Most of these signals will initially
  be received and unfiltered through the appMessage() signal.

  This class also provides a set of useful static functions. The
  qpeDir() and documentDir() functions return the respective paths.
  The grabKeyboard() and ungrabKeyboard() functions are used to
  control whether the application takes control of the device's
  physical buttons (e.g. application launch keys). The stylus' mode of
  operation is set with setStylusOperation() and retrieved with
  stylusOperation(). There are also setInputMethodHint() and
  inputMethodHint() functions.

  \ingroup qtopiaemb
*/

/*!
    \enum QPEApplication::screenSaverHint

    \value Disable the screen should never blank
    \value DisableLightOff the screen should never blank and the
    backlight should never switch off
    \value DisableSuspend the screen should blank when the device goes
    into suspend mode
    \value Enable screen blanking and switching off the backlight are
    both enabled

    Currently, this is only used internally.

*/

/*!
  \fn void QPEApplication::clientMoused()

  \internal
*/

/*!
  \fn void QPEApplication::timeChanged();

  This signal is emitted when the time changes outside the normal
  passage of time, i.e. if the time is set backwards or forwards.

  If the application offers the TimeMonitor service, it will get
  the QCop message that causes this signal even if it is not running,
  thus allowing it to update any alarms or other time-related records.
*/

/*!
  \fn void QPEApplication::clockChanged( bool ampm );

  This signal is emitted when the user changes the clock's style. If
  \a ampm is TRUE, the user wants a 12-hour AM/PM clock, otherwise,
  they want a 24-hour clock.
*/

/*!
    \fn void QPEApplication::volumeChanged( bool muted )

    This signal is emitted whenever the mute state is changed. If \a
    muted is TRUE, then sound output has been muted.
*/

/*!
    \fn void QPEApplication::weekChanged( bool startOnMonday )

    This signal is emitted if the week start day is changed. If \a
    startOnMonday is TRUE then the first day of the week is Monday; if
    \a startOnMonday is FALSE then the first day of the week is
    Sunday.
*/

/*!
    \fn void QPEApplication::dateFormatChanged( DateFormat )

    This signal is emitted whenever the date format is changed.
*/

/*!
    \fn void QPEApplication::flush()

    \internal
*/

/*!
    \fn void QPEApplication::reload()

    \internal
*/

/*!
  \fn void QPEApplication::appMessage( const QCString& msg, const QByteArray& data )

  This signal is emitted when a message is received on this
  application's QPE/Application/<i>appname</i> \link qcop.html
  QCop\endlink channel.

  The slot to which you connect this signal uses \a msg and \a data
  in the following way:

\code
    void MyWidget::receive( const QCString& msg, const QByteArray& data )
    {
	QDataStream stream( data, IO_ReadOnly );
	if ( msg == "someMessage(int,int,int)" ) {
	    int a,b,c;
	    stream >> a >> b >> c;
	    ...
	} else if ( msg == "otherMessage(QString)" ) {
	    ...
	}
    }
\endcode

   Note that messages received here may be processed by qpe application
   and emitted as signals, such as flush() and reload().
*/

/*!
  Constructs a QPEApplication just as you would construct
  a QApplication, passing \a argc, \a argv, and \a t.

  For applications, \a t should be the default, GuiClient. Only
  the Qtopia server passes GuiServer.
*/
QPEApplication::QPEApplication( int& argc, char **argv, Type t )
    : QApplication( hack(argc), argv, t )
{
    d = new QPEApplicationData;
    d->loadTextCodecs();
    d->loadImageCodecs();

    int dw = desktop()->width();
    if ( dw < 200 ) {
	setFont( QFont( "helvetica", 8 ) );
	AppLnk::setSmallIconSize(10);
	AppLnk::setBigIconSize(28);
    } else if ( dw > 600 ) {
	setFont( QFont( "helvetica", 12 ) );
	AppLnk::setSmallIconSize(24);
	AppLnk::setBigIconSize(48);	
    } else if ( dw > 400 ) {
	setFont( QFont( "helvetica", 12 ) );
	AppLnk::setSmallIconSize(16);
	AppLnk::setBigIconSize(32);	
    }

    QMimeSourceFactory::setDefaultFactory(new ResourceMimeFactory);

    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(hideOrQuit()));
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)

    QString qcopfn("/tmp/qcop-msg-");
    qcopfn += QString(argv[0]); // append command name

    QFile f(qcopfn);
    if ( f.open(IO_ReadOnly) ) {
	flock(f.handle(), LOCK_EX);
    }

    sysChannel = new QCopChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(const QCString &, const QByteArray &)),
	     this, SLOT(systemMessage( const QCString &, const QByteArray &)) );

    QCString channel = QCString(argv[0]);
    channel.replace(QRegExp(".*/"),"");
    d->appName = channel;
    channel = "QPE/Application/" + channel;
    pidChannel = new QCopChannel( channel, this);
    connect( pidChannel, SIGNAL(received(const QCString &, const QByteArray &)),
	    this, SLOT(pidMessage(const QCString &, const QByteArray &)));

    if ( f.isOpen() ) {
	d->keep_running = FALSE;
	QDataStream ds(&f);
	QCString channel, message;
	QByteArray data;
	while(!ds.atEnd()) {
	    ds >> channel >> message >> data;
	    d->enqueueQCop(channel,message,data);
	}

	flock(f.handle(), LOCK_UN);
	f.close();
	f.remove();
    }

    for (int a=0; a<argc; a++) {
	if ( qstrcmp(argv[a],"-preload")==0 ) {
	    argv[a] = argv[a+1];
	    a++;
	    d->preloaded = TRUE;
	    argc-=1;
	} else if ( qstrcmp(argv[a],"-preload-show")==0 ) {
	    argv[a] = argv[a+1];
	    a++;
	    d->preloaded = TRUE;
	    d->forceshow = TRUE;
	    argc-=1;
	}
    }

    /* overide stored arguments */
    setArgs(argc, argv);

#endif

    FontDatabase::loadRenderers();  // load font factory plugins.

#ifndef QT_NO_TRANSLATION
    QStringList langs = Global::languageList();
    for (QStringList::ConstIterator it = langs.begin(); it!=langs.end(); ++it) {
	QString lang = *it;

	QTranslator * trans;
	QString tfn;

	trans = new QTranslator(this);
	tfn = qpeDir()+"/i18n/"+lang+"/libqpe.qm";
	if ( trans->load( tfn ))
	    installTranslator( trans );
	else
	    delete trans;

	trans = new QTranslator(this);
	tfn = qpeDir()+"/i18n/"+lang+"/"+d->appName+".qm";
	if ( trans->load( tfn ))
	    installTranslator( trans );
	else
	    delete trans;

	//###language/font hack; should look it up somewhere
#ifdef QWS
	if ( lang == "ja" || lang == "zh_CN" || lang == "zh_TW" || lang == "ko" ) {
	    QFont fn = FontManager::unicodeFont( FontManager::Proportional );
	    setFont( fn );
	}
#endif
    }
#endif

    applyStyle();

    if ( type() == GuiServer ) {
	setScreenSaverInterval(-1);
	setVolume();
#ifdef QWS
	QWSServer::setScreenSaver(new QPEScreenSaver);
#endif
    }
    installEventFilter( this );

    QPEMenuToolFocusManager::initialize();

#ifdef QT_NO_QWS_CURSOR
    // if we have no cursor, probably don't want tooltips
    QToolTip::setEnabled( FALSE );
#endif
}

static QPtrDict<void>* inputMethodDict=0;
static void createInputMethodDict()
{
    if ( !inputMethodDict )
	inputMethodDict = new QPtrDict<void>;
}

/*!
  Returns the currently set hint to the system as to whether
  widget \a w has any use for text input methods.

  \sa setInputMethodHint() InputMethodHint
*/
QPEApplication::InputMethodHint QPEApplication::inputMethodHint( QWidget* w )
{
    if ( inputMethodDict && w )
	return (InputMethodHint)(int)inputMethodDict->find(w);
    return Normal;
}

/*!
    \enum QPEApplication::InputMethodHint

    \value Normal the application sometimes needs text input (the default).
    \value AlwaysOff the application never needs text input.
    \value AlwaysOn the application always needs text input.

    \sa inputMethodHint() setInputMethodHint()
*/

/*!
  Hints to the system that widget \a w has use for text input methods
  as specified by \a mode.

  \sa inputMethodHint() InputMethodHint
*/
void QPEApplication::setInputMethodHint( QWidget* w, InputMethodHint mode )
{
    createInputMethodDict();
    if ( mode == Normal ) {
	inputMethodDict->remove(w);
    } else {
	inputMethodDict->insert(w,(void*)mode);
    }
}

class HackDialog : public QDialog
{
public:
    void acceptIt() { accept(); }
    void rejectIt() { reject(); }
};


void QPEApplication::mapToDefaultAction( QWSKeyEvent *ke, int key )
{
    // specialised actions for certain widgets. May want to
    // add more stuff here.
    if ( activePopupWidget() && activePopupWidget()->inherits( "QListBox" )
	 && activePopupWidget()->parentWidget()
	 && activePopupWidget()->parentWidget()->inherits( "QComboBox" ) )
	key = Qt::Key_Return;

    if ( activePopupWidget() && activePopupWidget()->inherits( "QPopupMenu" ) )
	key = Qt::Key_Return;

#ifdef QWS
    ke->simpleData.keycode = key;
#endif
}

class HackWidget : public QWidget
{
public:
    bool needsOk()
    { return (getWState() & WState_Reserved1 ); }
};

#ifdef QWS
/*!
  Filters Qt events to implement Qtopia-specific functionality.
*/
bool QPEApplication::qwsEventFilter( QWSEvent *e )
{
    if ( !d->notbusysent && e->type == QWSEvent::Focus ) {
	if ( qApp->type() != QApplication::GuiServer ) {
	    QCopEnvelope e("QPE/System", "notBusy(QString)" );
	    e << d->appName;
	}
	d->notbusysent=TRUE;
    }
    if ( type() == GuiServer ) {
	switch ( e->type ) {
	    case QWSEvent::Mouse:
		if ( e->asMouse()->simpleData.state && !QWidget::find(e->window()) )
		    emit clientMoused();
	}
    }
    if ( e->type == QWSEvent::Key ) {
	if ( d->kbgrabber == 1 )
	    return TRUE;
	QWSKeyEvent *ke = (QWSKeyEvent *)e;
	if ( ke->simpleData.keycode == Qt::Key_F33 ) {
	    // Use special "OK" key to press "OK" on top level widgets
	    QWidget *active = activeWindow();
	    QWidget *popup = 0;
	    if ( active && active->isPopup() ) {
		popup = active;
		active = active->parentWidget();
	    }
	    if ( active && (int)active->winId() == ke->simpleData.window &&
		 !active->testWFlags( WStyle_Customize|WType_Popup|WType_Desktop )) {
		if ( ke->simpleData.is_press ) {
		    if ( popup )
			popup->close();
		    if ( active->inherits( "QDialog" ) ) {
			HackDialog *d = (HackDialog *)active;
			d->acceptIt();
			return TRUE;
		    } else if ( ((HackWidget *)active)->needsOk() ) {
			QSignal s;
			s.connect( active, SLOT( accept() ) );
			s.activate();
		    } else {
			// do the same as with the select key: Map to the default action of the widget:
			mapToDefaultAction( ke, Qt::Key_Return );
		    }
		}
	    }
	} else if ( ke->simpleData.keycode == Qt::Key_F30 ) {
	    // Use special "select" key to do whatever default action a widget has
	    mapToDefaultAction( ke, Qt::Key_Space );
	} else if ( ke->simpleData.keycode == Qt::Key_Escape &&
		    ke->simpleData.is_press ) {
	    // Escape key closes app if focus on toplevel
	    QWidget *active = activeWindow();
	    if ( active && active->testWFlags( WType_TopLevel ) &&
		 (int)active->winId() == ke->simpleData.window &&
		 !active->testWFlags( WStyle_Dialog|WStyle_Customize|WType_Popup|WType_Desktop )) {
		if ( active->inherits( "QDialog" ) ) {
		    HackDialog *d = (HackDialog *)active;
		    d->rejectIt();
		    return TRUE;
		} else if ( strcmp( argv()[0], "embeddedkonsole") != 0 ) {
		    active->close();
		}
	    }
	}

#if QT_VERSION < 231
    // Filter out the F4/Launcher key from apps
    // ### The launcher key may not always be F4 on all devices
	if ( ((QWSKeyEvent *)e)->simpleData.keycode == Qt::Key_F4 )
	    return TRUE;
#endif
    }
    if ( e->type == QWSEvent::Focus ) {
	QWSFocusEvent *fe = (QWSFocusEvent*)e;
	QWidget* nfw = QWidget::find(e->window());
	if ( !fe->simpleData.get_focus ) {
	    QWidget *active = activeWindow();
	    while ( active && active->isPopup() ) {
		active->close();
		active = activeWindow();
	    }
	    if ( !nfw && d->kbgrabber == 2 ) {
		ungrabKeyboard();
		d->kbregrab = TRUE; // want kb back when we're active
	    }
	} else {
	    // make sure our modal widget is ALWAYS on top
	    QWidget *topm = activeModalWidget();
	    if ( topm ) {
		topm->raise();
	    }
	    if ( d->kbregrab ) {
		grabKeyboard();
		d->kbregrab = FALSE;
	    }
	}
	if ( fe->simpleData.get_focus && inputMethodDict ) {
	    InputMethodHint m = inputMethodHint( QWidget::find(e->window()) );
	    if ( m == AlwaysOff )
		Global::hideInputMethod();
	    if ( m == AlwaysOn )
		Global::showInputMethod();
	}
    }
    return QApplication::qwsEventFilter( e );
}
#endif

/*!
  Destroys the QPEApplication.
*/
QPEApplication::~QPEApplication()
{
    if ( !d->notbusysent ) {
	// maybe we didn't map a window - still tell the server we're not
	// busy anymore.
	if ( qApp->type() != QApplication::GuiServer ) {
	    QCopEnvelope e("QPE/System", "notBusy(QString)" );
	    e << d->appName;
	}
    }
    ungrabKeyboard();
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    // Need to delete QCopChannels early, since the display will
    // be gone by the time we get to ~QObject().
    delete sysChannel;
    delete pidChannel;
#endif
    delete d;
}

/*!
  Returns \c{$QPEDIR/}.
*/
QString QPEApplication::qpeDir()
{
    const char *base = getenv( "QPEDIR" );
    if ( base )
	return QString( base ) + "/";

    return QString( "../" );
}

/*!
  Returns the user's current Document directory. There is a trailing "/".
*/
QString QPEApplication::documentDir()
{
    const char *base = getenv( "HOME" );
    if ( base )
	return QString( base ) + "/Documents/";

    return QString( "../Documents/" );
}

static int deforient=-1;

/*!
  \internal
*/
int QPEApplication::defaultRotation()
{
    if ( deforient < 0 ) {
	QString d = getenv("QWS_DISPLAY");
	if ( d.contains("Rot90") ) {
	    deforient = 90;
	} else if ( d.contains("Rot180") ) {
	    deforient = 180;
	} else if ( d.contains("Rot270") ) {
	    deforient = 270;
	} else {
	    deforient=0;
	}
    }
    return deforient;
}

/*!
  \internal
*/
void QPEApplication::setDefaultRotation(int r)
{
    if ( qApp->type() == GuiServer ) {
	deforient = r;
	setenv("QWS_DISPLAY", QString("Transformed:Rot%1:0").arg(r).latin1(), 1);
	Config config("qpe");
        config.setGroup( "Rotation" );
	config.writeEntry( "Screen", getenv("QWS_DISPLAY") );

    } else {
#ifndef QT_NO_COP
	QCopEnvelope("QPE/System", "setDefaultRotation(int)") << r;
#endif
    }
}

/*!
  \internal
*/
void QPEApplication::applyStyle()
{
    Config config( "qpe" );

    config.setGroup( "Appearance" );

    // Widget style
    QString style = config.readEntry( "Style", "Qtopia" );
    internalSetStyle( style );

    // Colors
    QColor bgcolor( config.readEntry( "Background", "#E5E1D5" ) );
    QColor btncolor( config.readEntry( "Button", "#D6CDBB" ) );
    QPalette pal( btncolor, bgcolor );
    QString color = config.readEntry( "Highlight", "#800000" );
    pal.setColor( QColorGroup::Highlight, QColor(color) );
    color = config.readEntry( "HighlightedText", "#FFFFFF" );
    pal.setColor( QColorGroup::HighlightedText, QColor(color) );
    color = config.readEntry( "Text", "#000000" );
    pal.setColor( QColorGroup::Text, QColor(color) );
    color = config.readEntry( "ButtonText", "#000000" );
    pal.setColor( QPalette::Active, QColorGroup::ButtonText, QColor(color) );
    color = config.readEntry( "Base", "#FFFFFF" );
    pal.setColor( QColorGroup::Base, QColor(color) );

    pal.setColor( QPalette::Disabled, QColorGroup::Text,
		  pal.color(QPalette::Active, QColorGroup::Background).dark() );

    setPalette( pal, TRUE );

    // Window Decoration
    QString dec = config.readEntry( "Decoration", "Qtopia" );
    if ( dec != d->decorationName ) {
	qwsSetDecoration( new QPEDecoration( dec ) );
	d->decorationName = dec;
    }

    // Font
    QString ff = config.readEntry( "FontFamily", font().family() );
    int fs = config.readNumEntry( "FontSize", font().pointSize() );
    setFont( QFont(ff,fs), TRUE );
}

void QPEApplication::systemMessage( const QCString &msg, const QByteArray &data)
{
#ifdef Q_WS_QWS
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "applyStyle()" ) {
	applyStyle();
    } else if ( msg == "setScreenSaverInterval(int)" ) {
	if ( type() == GuiServer ) {
	    int time;
	    stream >> time;
	    setScreenSaverInterval(time);
	}
    } else if ( msg == "setScreenSaverIntervals(int,int,int)" ) {
	if ( type() == GuiServer ) {
	    int t1,t2,t3;
	    stream >> t1 >> t2 >> t3;
	    setScreenSaverIntervals(t1,t2,t3);
	}
    } else if ( msg == "setBacklight(int)" ) {
	if ( type() == GuiServer ) {
	    int bright;
	    stream >> bright;
	    setBacklight(bright);
	}
    } else if ( msg == "setDefaultRotation(int)" ) {
	if ( type() == GuiServer ) {
	    int r;
	    stream >> r;
	    setDefaultRotation(r);
	}
    } else if ( msg == "shutdown()" ) {
	if ( type() == GuiServer )
	    shutdown();
    } else if ( msg == "quit()" ) {
	if ( type() != GuiServer )
	    tryQuit();
    } else if ( msg == "forceQuit()" ) {
	if ( type() != GuiServer )
	    quit();
    } else if ( msg == "restart()" ) {
	if ( type() == GuiServer )
	    restart();
    } else if ( msg == "grabKeyboard(QString)" ) {
	QString who;
	stream >> who;
	if ( who.isEmpty() )
	    d->kbgrabber = 0;
	else if ( who != d->appName )
	    d->kbgrabber = 1;
	else
	    d->kbgrabber = 2;
    } else if ( msg == "language(QString)" ) {
	if ( type() == GuiServer ) {
	    QString l;
	    stream >> l;
	    QString cl = getenv("LANG");
	    if ( cl != l ) {
		if ( l.isNull() )
		    unsetenv( "LANG" );
		else
		    setenv( "LANG", l.latin1(), 1 );
		restart();
	    }
	}
    } else if ( msg == "timeChange(QString)" ) {
	d->skiptimechanged++;
	QString t;
	stream >> t;
	if ( t.isNull() )
	    unsetenv( "TZ" );
	else
	    setenv( "TZ", t.latin1(), 1 );
	// emit the signal so everyone else knows...
	emit timeChanged();
    } else if ( msg == "execute(QString)" ) {
	if ( type() == GuiServer ) {
	    QString t;
	    stream >> t;
	    Global::execute( t );
	}
    } else if ( msg == "execute(QString,QString)" ) {
	if ( type() == GuiServer ) {
	    QString t,d;
	    stream >> t >> d;
	    Global::execute( t, d );
	}
    } else if ( msg == "addAlarm(QDateTime,QCString,QCString,int)" ) {
	if ( type() == GuiServer ) {
	    QDateTime when;
	    QCString channel, message;
	    int data;
	    stream >> when >> channel >> message >> data;
	    AlarmServer::addAlarm( when, channel, message, data );
	}
    } else if ( msg == "deleteAlarm(QDateTime,QCString,QCString,int)" ) {
	if ( type() == GuiServer ) {
	    QDateTime when;
	    QCString channel, message;
	    int data;
	    stream >> when >> channel >> message >> data;
	    AlarmServer::deleteAlarm( when, channel, message, data );
	}
    } else if ( msg == "clockChange(bool)" ) {
	int tmp;
	stream >> tmp;
	emit clockChanged( tmp );
    } else if ( msg == "weekChange(bool)" ) {
	int tmp;
	stream >> tmp;
	emit weekChanged( tmp );
    } else if ( msg == "setDateFormat(DateFormat)" ) {
	DateFormat tmp;
	stream >> tmp;
	emit dateFormatChanged( tmp );
    } else if ( msg == "setVolume(int,int)" ) {
	int t,v;
	stream >> t >> v;
	setVolume(t,v);
	emit volumeChanged( muted );
    } else if ( msg == "volumeChange(bool)" ) {
	stream >> muted;
	setVolume();
	emit volumeChanged( muted );
    } else if ( msg == "setScreenSaverMode(int)" ) {
	if ( type() == GuiServer ) {
	    int old = disable_suspend;
	    stream >> disable_suspend;
	    //qDebug("setScreenSaverMode(%d)", disable_suspend );
	    if ( disable_suspend > old )
		setScreenSaverInterval( -1 );
	}
    }
#endif
}

/*!
  \internal
*/
bool QPEApplication::raiseAppropriateWindow()
{
    bool r=FALSE;
    // ########## raise()ing main window should raise and set active
    // ########## it and then all childen. This belongs in Qt/Embedded
    QWidget *top = d->qpe_main_widget;
    if ( !top ) top =mainWidget();
    if ( top && d->keep_running ) {
	if ( top->isVisible() )
	    r = TRUE;
	else if (d->preloaded) {
	  // We are preloaded and not visible.. pretend we just started..
 	  QCopEnvelope e("QPE/System", "fastAppShowing(QString)");
 	  e << d->appName;
	}
	
	d->show_mx(top,d->nomaximize);
	top->raise();
	top->setActiveWindow();
    }
    QWidget *topm = activeModalWidget();
    if ( topm && topm != top ) {
	topm->show();
	topm->raise();
	topm->setActiveWindow();
	// If we haven't already handled the fastAppShowing message
	if (!top && d->preloaded) {
 	  QCopEnvelope e("QPE/System", "fastAppShowing(QString)");
 	  e << d->appName;
	}	
	r = FALSE;
    }
    return r;
}

void QPEApplication::pidMessage( const QCString &msg, const QByteArray & data)
{
#ifdef Q_WS_QWS

    if ( msg == "quit()" ) {
	tryQuit();
    } else if ( msg == "quitIfInvisible()" ) {
	if ( d->qpe_main_widget && !d->qpe_main_widget->isVisible() )
	    quit();
    } else if ( msg == "close()" ) {
	hideOrQuit();
    } else if ( msg == "disablePreload()" ) {
	d->preloaded = FALSE;
	d->keep_running = TRUE;
	/* so that quit will quit */
    } else if ( msg == "enablePreload()" ) {
      if (d->qpe_main_widget)
	d->preloaded = TRUE;
	d->keep_running = TRUE;
	/* so next quit won't quit */
    } else if ( msg == "raise()" ) {
	d->keep_running = TRUE;
	d->notbusysent = FALSE;
	raiseAppropriateWindow();
	// Tell the system we're still chugging along...
	QCopEnvelope e("QPE/System", "appRaised(QString)");
	e << d->appName;
    } else if ( msg == "flush()" ) {
	emit flush();
	// we need to tell the desktop
	QCopEnvelope e( "QPE/Desktop", "flushDone(QString)" );
	e << d->appName;
    } else if ( msg == "reload()" ) {
	emit reload();
    } else if ( msg == "setDocument(QString)" ) {
	d->keep_running = TRUE;
	QDataStream stream( data, IO_ReadOnly );
	QString doc;
	stream >> doc;
	QWidget *mw = mainWidget();
	if ( !mw )
	    mw = d->qpe_main_widget;
	if ( mw )
	    Global::setDocument( mw, doc );
    } else if ( msg == "timeChange(QString)" ) {
	// Same as the QPE/System message
	if ( d->skiptimechanged ) {
	    // Was handled in pidMessage()
	    // (ie. we must offer TimeMonitor service)
	    --d->skiptimechanged;
	} else {
	    QDataStream stream( data, IO_ReadOnly );
	    QString t;
	    stream >> t;
	    if ( t.isNull() )
		unsetenv( "TZ" );
	    else
		setenv( "TZ", t.latin1(), 1 );
	    // emit the signal so everyone else knows...
	    emit timeChanged();
	}
    } else {
	bool p = d->keep_running;
	d->keep_running = FALSE;
	emit appMessage( msg, data);
	if ( d->keep_running ) {
	    d->notbusysent = FALSE;
	    raiseAppropriateWindow();
	    if ( !p ) {
		// Tell the system we're still chugging along...
		QCopEnvelope e("QPE/System", "appRaised(QString)");
		e << d->appName;
	    }
	}
	if ( p )
	    d->keep_running = p;
    }
#endif
}



/*!
  Sets widget \a mw as the mainWidget() and shows it. For small windows,
  consider passing TRUE for \a nomaximize rather than the default FALSE.

  \sa showMainDocumentWidget()
*/
void QPEApplication::showMainWidget( QWidget* mw, bool nomaximize )
{
    d->show(mw,nomaximize);
}

/*!
  Sets widget \a mw as the mainWidget() and shows it. For small windows,
  consider passing TRUE for \a nomaximize rather than the default FALSE.

  This calls designates the application as
  a \link docwidget.html document-oriented\endlink application.

  The \a mw widget \e must have this slot: setDocument(const QString&).

  \sa showMainWidget()
*/
void QPEApplication::showMainDocumentWidget( QWidget* mw, bool nomaximize )
{
    if ( mw && argc() == 2 )
	Global::setDocument( mw, QString::fromUtf8(argv()[1]) );

    d->show(mw,nomaximize);
}


/*!
    If an application is started via a \link qcop.html QCop\endlink
    message, the application will process the \link qcop.html
    QCop\endlink message and then quit. If the application calls this
    function while processing a \link qcop.html QCop\endlink message,
    after processing its outstanding \link qcop.html QCop\endlink
    messages the application will start 'properly' and show itself.

  \sa keepRunning()
*/
void QPEApplication::setKeepRunning()
{
    if ( qApp && qApp->inherits( "QPEApplication" ) ) {
	QPEApplication *qpeApp = (QPEApplication*)qApp;
	qpeApp->d->keep_running = TRUE;
    }
}

/*!
  Returns TRUE if the application will quit after processing the
  current list of qcop messages; otherwise returns FALSE.

  \sa setKeepRunning()
*/
bool QPEApplication::keepRunning() const
{
    return d->keep_running;
}

/*!
  \internal
*/
void QPEApplication::internalSetStyle( const QString &style )
{
    if ( style == d->styleName )
	return;

    QStyle *newStyle = 0;

#if QT_VERSION >= 300
    if ( style == "QPE"  || style == "Qtopia" ) {
	newStyle = new QPEStyle;
    } else {
	newStyle = QStyleFactory::create(style);
    }
#else
    StyleInterface *oldIface = d->styleIface;
    QLibrary *oldLib = d->styleLib;
    d->styleIface = 0;
    d->styleLib = 0;

    if ( style == "Windows" ) {
	newStyle = new QWindowsStyle;
    } else if ( style == "QPE" || style == "Qtopia" ) {
	newStyle = new QPEStyle;
    } else if ( style.findRev( ".so" ) == (int)style.length()-3 ) {
	QString path = QPEApplication::qpeDir() + "/plugins/styles";
	StyleInterface *iface = 0;
	QLibrary *lib = new QLibrary( path + "/" + style );
	if ( lib->queryInterface( IID_Style, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
	    newStyle = iface->style();
	    d->styleIface = iface;
	    d->styleLib = lib;
	} else {
	    lib->unload();
	    delete lib;
	}
    } 
#endif

    if ( !newStyle ) {
	newStyle = new QPEStyle;
	d->styleName = "QPE";
    } else {
	d->styleName = style;
    }
    setStyle( newStyle );

#if QT_VERSION < 300
    if ( oldIface ) {
	// cleanup old plugin.
	oldIface->release();
	oldLib->unload();
	delete oldLib;
    }
#endif
}

/*!
  \internal
*/
void QPEApplication::prepareForTermination(bool willrestart)
{
    if ( willrestart ) {
	// Draw a big wait icon, the image can be altered in later revisions
//	QWidget *d = QApplication::desktop();
	QPixmap pix = Resource::loadPixmap("bigwait");
	QLabel *lblWait = new QLabel(0, "wait hack!", QWidget::WStyle_Customize |
				  QWidget::WStyle_NoBorder | QWidget::WStyle_Tool );
	lblWait->setPixmap( pix );
	lblWait->setAlignment( QWidget::AlignCenter );
	lblWait->show();
	lblWait->showMaximized();
    }
#ifndef SINGLE_APP
#ifndef QT_NO_COP
    { QCopEnvelope envelope("QPE/System", "forceQuit()"); }
#endif
    processEvents(); // ensure the message goes out.
    sleep(1); // You have 1 second to comply.
#endif
}

/*!
  \internal
*/
void QPEApplication::shutdown()
{
    // Implement in server's QPEApplication subclass
}

/*!
  \internal
*/
void QPEApplication::restart()
{
    // Implement in server's QPEApplication subclass
}

static QPtrDict<void>* stylusDict=0;
static void createDict()
{
    if ( !stylusDict )
	stylusDict = new QPtrDict<void>;
}

/*!
  Returns the current StylusMode for widget \a w.

  \sa setStylusOperation() StylusMode
*/
QPEApplication::StylusMode QPEApplication::stylusOperation( QWidget* w )
{
    if ( stylusDict )
	return (StylusMode)(int)stylusDict->find(w);
    return LeftOnly;
}

/*!
    \enum QPEApplication::StylusMode

    \value LeftOnly the stylus only generates LeftButton
			events (the default).
    \value RightOnHold the stylus generates RightButton events
			if the user uses the press-and-hold gesture.

    \sa setStylusOperation() stylusOperation()
*/

/*!
  Causes widget \a w to receive mouse events according to the stylus
  \a mode.

  \sa stylusOperation() StylusMode
*/
void QPEApplication::setStylusOperation( QWidget* w, StylusMode mode )
{
    createDict();
    if ( mode == LeftOnly ) {
	stylusDict->remove(w);
	w->removeEventFilter(qApp);
    } else {
	stylusDict->insert(w,(void*)mode);
	connect(w,SIGNAL(destroyed()),qApp,SLOT(removeSenderFromStylusDict()));
	w->installEventFilter(qApp);
    }
}


/*!
  \reimp
*/
bool QPEApplication::eventFilter( QObject *o, QEvent *e )
{
    if ( stylusDict && e->type() >= QEvent::MouseButtonPress && e->type() <= QEvent::MouseMove ) {
	QMouseEvent* me = (QMouseEvent*)e;
	StylusMode mode = (StylusMode)(int)stylusDict->find(o);
	switch (mode) {
	  case RightOnHold:
	    switch ( me->type() ) {
	      case QEvent::MouseButtonPress:
		if ( me->button() == LeftButton ) {
		    d->presstimer = startTimer(500); // #### pref.
		    d->presswidget = (QWidget*)o;
		    d->presspos = me->pos();
		    d->rightpressed = FALSE;
		}
		break;
	      case QEvent::MouseMove:
		if (d->presstimer && (me->pos()-d->presspos).manhattanLength() > 8) {
		    killTimer(d->presstimer);
		    d->presstimer = 0;
		}
		break;
	      case QEvent::MouseButtonRelease:
		if ( me->button() == LeftButton ) {
		    if ( d->presstimer ) {
			killTimer(d->presstimer);
			d->presstimer = 0;
		    }
		    if ( d->rightpressed && d->presswidget ) {
			// Right released
			postEvent( d->presswidget,
			    new QMouseEvent( QEvent::MouseButtonRelease, me->pos(),
				    RightButton, LeftButton+RightButton ) );
			// Left released, off-widget
			postEvent( d->presswidget,
			    new QMouseEvent( QEvent::MouseMove, QPoint(-1,-1),
				    LeftButton, LeftButton ) );
			postEvent( d->presswidget,
			    new QMouseEvent( QEvent::MouseButtonRelease, QPoint(-1,-1),
				    LeftButton, LeftButton ) );
			d->rightpressed = FALSE;
			return TRUE; // don't send the real Left release
		    }
		}
		break;
	      default:
		break;
	    }
	    break;
	  default:
	    ;
	}
    } else if ( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease ) {
	QKeyEvent *ke = (QKeyEvent *)e;
	if ( ke->key() == Key_Enter ) {
	    if ( o->isA( "QRadioButton" ) || o->isA( "QCheckBox" ) ) {
		postEvent( o, new QKeyEvent( e->type(), Key_Space, ' ',
		    ke->state(), " ", ke->isAutoRepeat(), ke->count() ) );
		return TRUE;
	    }
	}
    }

    return FALSE;
}

/*!
  \reimp
*/
void QPEApplication::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == d->presstimer && d->presswidget ) {
	// Right pressed
	postEvent( d->presswidget,
	    new QMouseEvent( QEvent::MouseButtonPress, d->presspos,
			RightButton, LeftButton ) );
	killTimer( d->presstimer );
	d->presstimer = 0;
	d->rightpressed = TRUE;
    }
}

void QPEApplication::removeSenderFromStylusDict()
{
    stylusDict->remove((void*)sender());
    if ( d->presswidget == sender() )
	d->presswidget = 0;
}

/*!
  \internal
*/
bool QPEApplication::keyboardGrabbed() const
{
    return d->kbgrabber;
}


/*!
  Reverses the effect of grabKeyboard(). This is called automatically
  on program exit.
*/
void QPEApplication::ungrabKeyboard()
{
    QPEApplicationData* d = ((QPEApplication*)qApp)->d;
    if ( d->kbgrabber == 2 ) {
#ifndef QT_NO_COP
	QCopEnvelope e("QPE/System", "grabKeyboard(QString)" );
	e << QString::null;
#endif
	d->kbregrab = FALSE;
	d->kbgrabber = 0;
    }
}

/*!
  Grabs the physical keyboard keys, e.g. the application's launching
  keys. Instead of launching applications when these keys are pressed
  the signals emitted are sent to this application instead. Some games
  programs take over the launch keys in this way to make interaction
  easier.

  \sa ungrabKeyboard()
*/
void QPEApplication::grabKeyboard()
{
    QPEApplicationData* d = ((QPEApplication*)qApp)->d;
    if ( qApp->type() == QApplication::GuiServer )
	d->kbgrabber = 0;
    else {
#ifndef QT_NO_COP
	QCopEnvelope e("QPE/System", "grabKeyboard(QString)" );
	e << d->appName;
#endif
	d->kbgrabber = 2; // me
    }
}

/*!
  \reimp
*/
int QPEApplication::exec()
{
#ifndef QT_NO_COP
    d->sendQCopQ();
#endif
    if ( d->keep_running)
	//|| d->qpe_main_widget && d->qpe_main_widget->isVisible() )
	return QApplication::exec();

#ifndef QT_NO_COP
    {
    QCopEnvelope e("QPE/System", "closing(QString)" );
    e << d->appName;
    }
#endif
    processEvents();
    return 0;
}

/*!
  \internal
  External request for application to quit.  Quits if possible without
  loosing state.
*/
void QPEApplication::tryQuit()
{
    if ( activeModalWidget() || strcmp( argv()[0], "embeddedkonsole") == 0 )
	return; // Inside modal loop or konsole. Too hard to save state.
#ifndef QT_NO_COP
    {
       QCopEnvelope e("QPE/System", "closing(QString)" );
       e << d->appName;
    }
#endif
    processEvents();

    quit();
}

/*!
  \internal
  User initiated quit.  Makes the window 'Go Away'.  If preloaded this means
  hiding the window.  If not it means quitting the application.
  As this is user initiated we don't need to check state.
*/
void QPEApplication::hideOrQuit()
{
    processEvents();

    // If we are a preloaded application we don't actually quit, so emit
    // a System message indicating we're quasi-closing.
    if ( d->preloaded && d->qpe_main_widget )
#ifndef QT_NO_COP
      {
	QCopEnvelope e("QPE/System", "fastAppHiding(QString)" );
	e << d->appName;
	d->qpe_main_widget->hide();
      }
#endif
    else
	quit();
}

/*!
    \fn void QPEApplication::showDialog( QDialog* dialog, bool nomax )

    Shows \a dialog. An heuristic approach is taken to
    determine the size and maximization of the dialog.

    \a nomax forces it to not be maximized.
*/

/*!
    \fn int QPEApplication::execDialog( QDialog* dialog, bool nomax )

    Shows and calls exec() on \a dialog. An heuristic approach is taken to
    determine the size and maximization of the dialog.

    \a nomax forces it to not be maximized.
*/

#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)

// The libraries with the skiff package (and possibly others) have
// completely useless implementations of builtin new and delete that
// use about 50% of your CPU. Here we revert to the simple libc
// functions.

void* operator new[](size_t size)
{
    return malloc(size);
}

void* operator new(size_t size)
{
    return malloc(size);
}

void operator delete[](void* p)
{
    free(p);
}

void operator delete[](void* p, size_t /*size*/)
{
    free(p);
}

void operator delete(void* p)
{
    free(p);
}

void operator delete(void* p, size_t /*size*/)
{
    free(p);
}

#endif

#if ( QT_VERSION <= 230 ) && !defined(SINGLE_APP)
#include <qwidgetlist.h>
#ifdef QWS
#include <qgfx_qws.h>
extern QRect qt_maxWindowRect;
void qt_setMaxWindowRect(const QRect& r)
{
    qt_maxWindowRect = qt_screen->mapFromDevice(r,
	qt_screen->mapToDevice(QSize(qt_screen->width(),qt_screen->height())));
    // Re-resize any maximized windows
    QWidgetList* l = QApplication::topLevelWidgets();
    if ( l ) {
        QWidget *w = l->first();
        while ( w ) {
            if ( w->isVisible() && w->isMaximized() )
            {
                w->showMaximized();
            }
            w = l->next();
        }
        delete l;
    }
}
#endif
#endif
