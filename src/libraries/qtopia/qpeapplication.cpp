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
#ifndef QTOPIA_INTERNAL_FILEOPERATIONS
#define QTOPIA_INTERNAL_FILEOPERATIONS
#endif
#include "custom.h"
#include <stdlib.h>
#include <qfile.h>

#ifdef QTOPIA_DESKTOP
#  include <qsettings.h>
#endif

#ifdef Q_WS_QWS
#  ifndef QT_NO_COP
#    if QT_VERSION <= 231
#      define private public
#      define sendLocally processEvent
#      include "qcopenvelope_qws.h"
#      undef private
#    else
#      include "qcopenvelope_qws.h"
#    endif
#  endif
#  include <qwindowsystem_qws.h>
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
#include <qclipboard.h>
#include <qtimer.h>
#include <qpixmapcache.h>

#ifdef Q_WS_QWS
#include <qwsdisplay_qws.h>
#endif

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#define QTOPIA_INTERNAL_INITAPP
#endif

#include "qpeapplication.h"
#if QT_VERSION >= 0x030000
#  include <qstylefactory.h>
#else
#  include <qwindowsstyle.h>
#endif
#include "global.h"
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
#  include "qutfcodec.h"
#endif
#include "config.h"
#include "network.h"
#  include "applnk.h"
#ifdef Q_WS_QWS
#  include "fontmanager.h"
#  include "fontdatabase.h"
#  include "power.h"
#  include "qpemenubar.h"
#  include "imagecodecinterface.h"
#  include "textcodecinterface.h"
#  include "styleinterface.h"
#  include "resource.h"
#  include "qpestyle.h"
#  include "qpedecoration_p.h"
#endif
#include "alarmserver.h"
#include "pluginloader_p.h"

#include <stdlib.h>

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <sys/file.h>
#endif

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

#ifndef QTOPIA_DESKTOP
#ifndef Q_OS_WIN32
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#else
#include <winbase.h>
#include <mmsystem.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#endif

#include "../qtopia1/qpe_show_dialog.cpp"

class HackWidget : public QWidget
{
public:
    bool needsOk()
    { return (getWState() & WState_Reserved1 ); }

    QRect normalGeometry()
    { return topData()->normalGeometry; };
};

#ifdef Q_WS_QWS
extern Q_EXPORT QRect qt_maxWindowRect;
#endif

class QPEApplicationData {
public:
    QPEApplicationData() : presstimer(0), presswidget(0), rightpressed(FALSE),
	kbgrabber(0), kbregrab(FALSE), notbusysent(FALSE), preloaded(FALSE),
	forceshow(FALSE), nomaximize(FALSE), qpe_main_widget(0),
	keep_running(TRUE), skiptimechanged(FALSE), execCalled(FALSE)
#ifdef Q_WS_QWS
	,styleLoader("styles"), styleIface(0), textPluginLoader("textcodecs"), // No tr
	imagePluginLoader("imagecodecs")
#endif
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
	    channel(ch), message(msg), data(d)
	    {
		channel.detach();
		message.detach();
		data.detach();
	    }

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
    bool execCalled;
    QList<QCopRec> qcopq;

    void enqueueQCop(const QCString &ch, const QCString &msg,
                               const QByteArray &data)
    {
	qcopq.append(new QCopRec(ch,msg,data));
    }
    void sendQCopQ()
    {
	if ( execCalled ) {
#ifndef QT_NO_COP
	    QCopRec* r;
	    for (QListIterator<QCopRec> it(qcopq); (r=it.current()); ++it)
		QCopChannel::sendLocally(r->channel,r->message,r->data);
#endif
	    qcopq.clear();
	}
    }

    static void show_mx(QWidget* mw, bool nomaximize, QString &strName)
    {
	if ( mw->isVisible() ) {
	    mw->raise();
	} else {
	    QPoint p;
	    QSize s;

	    if ( mw->layout() && mw->inherits("QDialog") ) {
		bool max;
		if ( read_widget_rect(strName, max, p, s) && validate_widget_size(mw, p, s) ) {
		    mw->resize(s);
		    mw->move(p);

		    if ( max && !nomaximize )
			mw->showMaximized();
		    else
			mw->show();
		} else {
		    qpe_show_dialog((QDialog*)mw,nomaximize);
		}
	    } else {
		bool max;
		if ( read_widget_rect(strName, max, p, s) && validate_widget_size(mw, p, s) ) {
		    mw->resize(s);
		    mw->move(p);
		} else {    //no stored rectangle, make an estimation
		    int x = (qApp->desktop()->width()-mw->frameGeometry().width())/2;
		    int y = (qApp->desktop()->height()-mw->frameGeometry().height())/2;
		    mw->move( QMAX(x,0), QMAX(y,0) );
#ifdef Q_WS_QWS
		    if ( !nomaximize )
			mw->showMaximized();
#endif
		}
		if ( max && !nomaximize )
		    mw->showMaximized();
		else
		    mw->show();
	    }
	}
    }

    static bool read_widget_rect(const QString &app, bool &maximized, QPoint &p, QSize &s)
    {
	maximized = TRUE;

	// 350 is the trigger in qwsdefaultdecoration for providing a resize button
	if ( qApp->desktop()->width() <= 350 )
	    return FALSE;

	Config cfg( "qpe" );
	cfg.setGroup("ApplicationPositions");
	QString str = cfg.readEntry( app, QString::null );
	QStringList l = QStringList::split(",", str);

	if ( l.count() == 5) {
	    p.setX( l[0].toInt() );
	    p.setY( l[1].toInt() );

	    s.setWidth( l[2].toInt() );
	    s.setHeight( l[3].toInt() );

	    maximized = l[4].toInt();

	    return TRUE;
	}

	return FALSE;
    }

    static bool validate_widget_size(const QWidget *w, QPoint &p, QSize &s)
    {
#ifndef Q_WS_QWS
	QRect qt_maxWindowRect = qApp->desktop()->geometry();
#endif
	int maxX = qt_maxWindowRect.width();
	int maxY = qt_maxWindowRect.height();
	int wWidth = s.width() + ( w->frameGeometry().width() - w->geometry().width() );
	int wHeight = s.height() + ( w->frameGeometry().height() - w->geometry().height() );

	// total window size is not allowed to be larger than desktop window size
	if ( ( wWidth >= maxX ) && ( wHeight >= maxY ) )
	    return FALSE;

	if ( wWidth > maxX ) {
	    s.setWidth( maxX - (w->frameGeometry().width() - w->geometry().width() ) );
	    wWidth = maxX;
	}

	if ( wHeight > maxY ) {
	    s.setHeight( maxY - (w->frameGeometry().height() - w->geometry().height() ) );
	    wHeight = maxY;
	}

	// any smaller than this and the maximize/close/help buttons will be overlapping
	if ( wWidth < 80 || wHeight < 60 )
	    return FALSE;

	if ( p.x() < 0 )
	    p.setX(0);
	if ( p.y() < 0 )
	    p.setY(0);

	if ( p.x() + wWidth > maxX )
	    p.setX( maxX - wWidth );
	if ( p.y() + wHeight > maxY )
	    p.setY( maxY - wHeight );

	return TRUE;
    }

    static void store_widget_rect(QWidget *w, QString &app)
    {
	// 350 is the trigger in qwsdefaultdecoration for providing a resize button
	if ( qApp->desktop()->width() <= 350 )
	    return;

	// we use these to map the offset of geometry and pos.  ( we can only use normalGeometry to
	// get the non-maximized version, so we have to do it the hard way )
	int offsetX = w->x() - w->geometry().left();
	int offsetY = w->y() - w->geometry().top();

	QRect r;
	if ( w->isMaximized() )
	    r = ( (HackWidget *) w)->normalGeometry();
	else
	    r = w->geometry();

	// Stores the window placement as pos(), size()  (due to the offset mapping)
	Config cfg( "qpe" );
	cfg.setGroup("ApplicationPositions");
	QString s;
	s.sprintf("%d,%d,%d,%d,%d", r.left() + offsetX, r.top() + offsetY, r.width(), r.height(), w->isMaximized() );
	cfg.writeEntry( app, s );
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
		mw->setIcon( (*it)->pixmap() );
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
		show_mx(mw,nomax, appName);
	} else if ( keep_running ) {
	    show_mx(mw,nomax, appName);
	}
    }

#ifdef Q_WS_QWS
    void loadTextCodecs()
    {
	QStringList list = textPluginLoader.list();
	QStringList::Iterator it;
	for ( it = list.begin(); it != list.end(); ++it ) {
	    TextCodecInterface *iface = 0;
	    if ( textPluginLoader.queryInterface( *it, IID_QtopiaTextCodec, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
		QValueList<int> mibs = iface->mibEnums();
		for (QValueList<int>::ConstIterator i=mibs.begin(); i!=mibs.end(); ++i) {
		    (void)iface->createForMib(*i);
		    // ### it exists now; need to remember if we can delete it
		}
	    }
	}
    }

    void loadImageCodecs()
    {
	QStringList list = imagePluginLoader.list();
	QStringList::Iterator it;
	for ( it = list.begin(); it != list.end(); ++it ) {
	    ImageCodecInterface *iface = 0;
	    if ( imagePluginLoader.queryInterface( *it, IID_QtopiaImageCodec, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
		QStringList formats = iface->keys();
		for (QStringList::ConstIterator i=formats.begin(); i!=formats.end(); ++i) {
		    (void)iface->installIOHandler(*i);
		    // ### it exists now; need to remember if we can delete it
		}
	    }
	}
    }
#endif

#ifdef Q_WS_QWS
    PluginLoaderIntern styleLoader;
    StyleInterface *styleIface;
    QString styleName;
    PluginLoaderIntern textPluginLoader;
    PluginLoaderIntern imagePluginLoader;
#endif
    QString decorationName;
};


class ResourceMimeFactory : public QMimeSourceFactory {
public:
    ResourceMimeFactory() : resImage(0)
    {
#ifdef Q_WS_QWS
	setFilePath( Global::helpPath() );
	setExtensionType("html","text/html;charset=UTF-8");
#endif
    }

    ~ResourceMimeFactory()
    {
	delete resImage;
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
#ifdef Q_WS_QWS
		QImage img = Resource::loadImage(name);
		if ( !img.isNull() ) {
		    ResourceMimeFactory *that = (ResourceMimeFactory*)this;
		    delete that->resImage;
		    that->resImage = new QImageDrag( img );
		    r = resImage;
		}
#endif
	    } while (!r && sl>0);
	}
	return r;
    }

private:
    QImageDrag *resImage;
};

int qtopia_muted=0;

#ifndef QTOPIA_DESKTOP
static void setVolume(int t=0, int percent=-1)
{
    switch (t) {
	case 0: {
	    Config cfg("Sound");
	    cfg.setGroup("System");
	    if ( percent < 0 )
		percent = cfg.readNumEntry("Volume",50);
	    int fd = 0;
#ifndef Q_OS_WIN32
	    if ((fd = open("/dev/mixer", O_RDWR))>=0) { // Some devices require this, O_RDONLY doesn't always work
		int vol = qtopia_muted ? 0 : percent;
		// set both channels to same volume
		vol |= vol << 8;
		ioctl(fd, MIXER_WRITE(0), &vol);
		::close(fd);
	    }
#else
	    HWAVEOUT handle;
	    WAVEFORMATEX formatData;
	    formatData.cbSize = sizeof(WAVEFORMATEX);
	    formatData.wFormatTag = WAVE_FORMAT_PCM;
	    formatData.nAvgBytesPerSec = 4 * 44000;
	    formatData.nBlockAlign = 4;
	    formatData.nChannels = 2;
	    formatData.nSamplesPerSec = 44000;
	    formatData.wBitsPerSample = 16;
	    waveOutOpen(&handle, WAVE_MAPPER, &formatData, 0L, 0L, CALLBACK_NULL);
	    int vol = qtopia_muted ? 0 : percent;
	    unsigned int volume = (vol << 24) | (vol << 8);
	    if ( waveOutSetVolume( handle, volume ) )
		qDebug( "set volume of audio device failed" );
	    waveOutClose( handle );
#endif
	} break;
    }
}
#endif


static int& hack(int& i)
{
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
    // These should be created, but aren't in Qt 2.3.0
    (void)new QUtf8Codec;
    (void)new QUtf16Codec;
#endif
    return i;
}

#ifdef Q_WS_QWS
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

void qpe_setBrightness(int bright);

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
    qpe_setBrightness(bright);
    curbl = bright;
}

void qpe_setBacklight(int bright) { setBacklight(bright); }
#endif

static bool dim_on = FALSE;
static bool lightoff_on = FALSE;
static bool networkedsuspend_on = FALSE;
static bool poweredsuspend_on = FALSE;

#ifdef Q_WS_QWS
static int disable_suspend = 100;

static bool powerOnlineStopsSuspend()
{
    return !poweredsuspend_on && PowerStatusManager::readStatus().acStatus() == PowerStatus::Online;
}

static bool networkOnlineStopsSuspend()
{
    return !networkedsuspend_on && Network::networkOnline();
}

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
    i1 = ssi(i1, config, "Dim","Interval_Dim", 30); // No tr
    i2 = ssi(i2, config, "LightOff","Interval_LightOff", 20);
    i3 = ssi(i3, config, "","Interval", 60); // No tr

    //qDebug("screen saver intervals: %d %d %d", i1, i2, i3);

    v[0] = QMAX( 1000*i1, 100);
    v[1] = QMAX( 1000*i2, 100);
    v[2] = QMAX( 1000*i3, 100);
    v[3] = 0;
    dim_on = ( (i1 != 0) ? config.readNumEntry("Dim",1) : FALSE );
    lightoff_on = ( (i2 != 0 ) ? config.readNumEntry("LightOff",1) : FALSE );
    poweredsuspend_on = config.readNumEntry("Suspend",0);
    networkedsuspend_on = config.readNumEntry("NetworkedSuspend",1);
#ifdef Q_WS_QWS
    if ( !i1 && !i2 && !i3 )
	QWSServer::setScreenSaverInterval(0);
    else
	QWSServer::setScreenSaverIntervals(v);
#endif
}

#ifndef QTOPIA_DESKTOP
static void setScreenSaverInterval(int interval)
{
    setScreenSaverIntervals(-1,-1,interval);
}
#endif


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
  \fn void QPEApplication::categoriesChanged();

  This signal is emitted whenever a category is added, removed or edited.
  Note, on Qtopia 1.5.0, this signal is never emitted.
*/

/*!
  \fn void QPEApplication::linkChanged( const QString &linkFile );

  This signal is emitted whenever an AppLnk or DocLnk is stored, removed or edited.
  \a linkFile contains the name of the link that is being modified.
*/

/*!
  \fn void QPEApplication::clockChanged( bool ampm );

  This signal is emitted when the user changes the clock's style. If
  \a ampm is TRUE, the user wants a 12-hour AM/PM clock, otherwise,
  they want a 24-hour clock.

  \warning if you use the TimeString functions, you should use
  TimeString::connectChange() instead.

  \sa dateFormatChanged()
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

    \warning if you use the TimeString functions, you should use
    TimeString::connectChange() instead.

    \sa clockChanged()
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

void QPEApplication::processQCopFile()
{
#ifndef QTOPIA_DESKTOP
#ifdef Q_OS_UNIX
    QString qcopfn(Global::tempDir()+ "qcop-msg-");
    qcopfn += d->appName; // append command name
#else
    QString qcopfn(Global::tempDir() + "qcop-msg-");
    if (QApplication::winVersion() != Qt::WV_98)
	qcopfn += d->appName; // append command name
    else
	qcopfn += d->appName.lower(); // append command name
#endif

    QFile f(qcopfn);
    if ( f.open(IO_ReadWrite) ) {
#ifndef Q_OS_WIN32
	flock(f.handle(), LOCK_EX);
#endif
	QDataStream ds(&f);
	QCString channel, message;
	QByteArray data;
	while(!ds.atEnd()) {
	    ds >> channel >> message >> data;
	    d->enqueueQCop(channel,message,data);
	}
	Global::truncateFile(f, 0);
#ifndef Q_OS_WIN32
	f.flush();
	flock(f.handle(), LOCK_UN);
#endif
    }
#endif
}

/*!
  Constructs a QPEApplication just as you would construct
  a QApplication, passing \a argc, \a argv, and \a t.

  For applications, \a t should be the default, GuiClient. Only
  the Qtopia server passes GuiServer.
*/
QPEApplication::QPEApplication( int& argc, char **argv, Type t )
    : QApplication( hack(argc), argv, t )
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
, pidChannel(0)
#endif
{
    QPixmapCache::setCacheLimit(256);  // sensible default for smaller devices.
    d = new QPEApplicationData;
#ifdef Q_WS_QWS
    PluginLoaderIntern::init();
    d->loadTextCodecs();
    d->loadImageCodecs();
#endif

    int dw = desktop()->width();
    if ( dw < 200 ) {
	setFont( QFont( "helvetica", 8 ) );
#ifdef Q_WS_QWS
	AppLnk::setSmallIconSize(10);
	AppLnk::setBigIconSize(28);
#endif
    } else if ( dw > 600 ) {
	setFont( QFont( "helvetica", 12 ) );
#ifdef Q_WS_QWS
	AppLnk::setSmallIconSize(24);
	AppLnk::setBigIconSize(48);
#endif
    } else if ( dw > 400 ) {
	setFont( QFont( "helvetica", 12 ) );
#ifndef Q_WS_QWS
	AppLnk::setSmallIconSize(16);
	AppLnk::setBigIconSize(32);
#endif
    }

    QMimeSourceFactory::setDefaultFactory(new ResourceMimeFactory);

    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(hideOrQuit()));

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)

    sysChannel = new QCopChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(const QCString &, const QByteArray &)),
	     this, SLOT(systemMessage( const QCString &, const QByteArray &)) );
    
#if 0
#ifdef Q_OS_UNIX
    QCString channel = QCString(argv[0]);
#else
    QCString channel;
    if (QApplication::winVersion() != Qt::WV_98)
	channel += QString(argv[0]); // append command name
    else
	channel += QString(argv[0]).lower(); // append command name
#endif
    channel.replace(QRegExp(".*/"),"");
    d->appName = channel;
    channel = "QPE/Application/" + channel;
    pidChannel = new QCopChannel( channel, this);
    connect( pidChannel, SIGNAL(received(const QCString &, const QByteArray &)),
	    this, SLOT(pidMessage(const QCString &, const QByteArray &)));

    processQCopFile();
    d->keep_running = d->qcopq.isEmpty();

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
#else
    initApp( argc, argv );
#endif // #if 0

#endif
#if defined(Q_WS_QWS)
    FontDatabase::loadRenderers();  // load font factory plugins.
#endif

#ifndef QT_NO_TRANSLATION
    QStringList qms;
    qms << "libqpe";
    qms << "libqtopia";
    qms << d->appName;

    QStringList langs = Global::languageList();

    for (QStringList::ConstIterator it = langs.begin(); it!=langs.end(); ++it) {
	QString lang = *it;

	QTranslator * trans;
	QString tfn;

	for (QStringList::ConstIterator qmit = qms.begin(); qmit!=qms.end(); ++qmit) {
	    trans = new QTranslator(this);
	    tfn = qpeDir() + "i18n/" + lang + "/" + *qmit + ".qm";
	    if ( trans->load( tfn ))
		installTranslator( trans );
	    else
		delete trans;
	}

	//###language/font hack; should look it up somewhere
#ifdef Q_WS_QWS
	if ( lang == "ja" || lang == "zh_CN" || lang == "zh_TW" || lang == "ko" ) {
	    QFont fn = FontManager::unicodeFont( FontManager::Proportional );
	    setFont( fn );
	}
#endif
    }
#endif

    applyStyle();

#ifndef QTOPIA_DESKTOP
    if ( type() == GuiServer ) {
	setScreenSaverInterval(-1);
	setVolume();
#ifdef Q_WS_QWS
	QWSServer::setScreenSaver(new QPEScreenSaver);
#endif
    }
#endif

    installEventFilter( this );

#ifdef Q_WS_QWS
    QPEMenuToolFocusManager::initialize();
#endif

#ifdef QT_NO_QWS_CURSOR
    // if we have no cursor, probably don't want tooltips
    QToolTip::setEnabled( FALSE );
#endif
}


#ifdef QTOPIA_INTERNAL_INITAPP
void QPEApplication::initApp( int argc, char **argv )
{
    delete pidChannel;
    d->keep_running = TRUE;
    d->preloaded = FALSE;
    d->forceshow = FALSE;

#ifdef Q_OS_UNIX
    QCString channel = QCString(argv[0]);
#else
    QCString channel;
    if (QApplication::winVersion() != Qt::WV_98)
	channel += QString(argv[0]); // append command name
    else
	channel += QString(argv[0]).lower(); // append command name
#endif
    channel.replace(QRegExp(".*/"),"");
    d->appName = channel;

    qt_fbdpy->setIdentity( channel ); // In Qt/E 2.3.6

    channel = "QPE/Application/" + channel;
    pidChannel = new QCopChannel( channel, this);
    connect( pidChannel, SIGNAL(received(const QCString &, const QByteArray &)),
	    this, SLOT(pidMessage(const QCString &, const QByteArray &)));

    processQCopFile();
    d->keep_running = d->qcopq.isEmpty();

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
}
#endif

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

#ifdef Q_WS_QWS
    ke->simpleData.keycode = key;
#endif
}

#ifdef Q_WS_QWS
/*!
  Filters Qt event \a e to implement Qtopia-specific functionality.
*/
bool QPEApplication::qwsEventFilter( QWSEvent *e )
{
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
    } else if ( e->type == QWSEvent::Focus ) {
	if ( !d->notbusysent ) {
	    if ( qApp->type() != QApplication::GuiServer ) {
#ifndef QT_NO_COP
		QCopEnvelope e("QPE/System", "notBusy(QString)" );
		e << d->appName;
#endif
	    }
	    d->notbusysent=TRUE;
	}

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
#ifndef QT_NO_COP
	    QCopEnvelope e("QPE/System", "notBusy(QString)" );
	    e << d->appName;
#endif
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
#ifdef Q_WS_QWS
    QString base, dir;
    if (getenv( "QPEDIR" ))
    	base = QString(getenv("QPEDIR")).stripWhiteSpace();
    if ( !base.isNull() && (base.length() > 0 )){
#ifdef Q_OS_WIN32
	QString temp(base);
	if (temp[(int)temp.length()-1] != QDir::separator())
	    temp.append(QDir::separator());
	dir = temp;
#else
	dir = QString( base ) + "/";
#endif
    }else{
	dir = QString( ".." ) + QDir::separator();
    }

    return dir;
#elif defined(QTOPIA_DESKTOP)

#ifdef __GNUG__
#warning "Should be able to change given a 'developing' parameter"
// qtopiadesktop has a 'debug' parameter, but we can't/shouldn't get it here.
#endif

    QSettings settings;
    settings.insertSearchPath( QSettings::Unix,
	    QDir::homeDirPath() + "/.palmtopcenter/" );
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QString key = "/palmtopcenter/qtopiadir";
    bool okay;
    QString dir = settings.readEntry(key, QString::null, &okay) + "/";
    if (!okay || dir.isNull()) {
	dir = "/opt/Qtopia/qtopiadesktop";
	settings.writeEntry( key, dir);
    }

    return dir;
#else
    qWarning("Cannot determine the install path");
    return QString::null;
#endif // Q_WS_QWS
}

/*!
  Returns the user's current Document directory. There is a trailing "/".
*/
QString QPEApplication::documentDir()
{
    QString r = QDir::homeDirPath();
#ifdef QTOPIA_DESKTOP
    r += "/.palmtopcenter/";
#endif

    QString base = r;
    if (base.length() > 0){
	return base + "/Documents/";
    }

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
	Config config("qpe");
        config.setGroup( "Rotation" );
	d = config.readEntry("Screen", d);
	if ( d.contains("Rot90") ) { // No tr
	    deforient = 90;
	} else if ( d.contains("Rot180") ) { // No tr
	    deforient = 180;
	} else if ( d.contains("Rot270") ) { // No tr
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

#ifdef Q_WS_QWS
    // Window Decoration
    QString dec = config.readEntry( "Decoration", "Qtopia" );
    if ( dec != d->decorationName ) {
	qwsSetDecoration( new QPEDecoration( dec ) );
	d->decorationName = dec;
    }
#endif

    // Font
    QString ff = config.readEntry( "FontFamily", font().family() );
    int fs = config.readNumEntry( "FontSize", font().pointSize() );
    QFont fn(ff,fs);

    // Icon size
#ifndef QPE_FONT_HEIGHT_TO_ICONSIZE
#define QPE_FONT_HEIGHT_TO_ICONSIZE(x) (x+1)
#endif
    int is = config.readNumEntry( "IconSize", -1 );
    if ( is < 0 ) {
	QFontMetrics fm(fn);
	config.writeEntry( "IconSize", QPE_FONT_HEIGHT_TO_ICONSIZE(fm.height()) );
	config.write();
    }

    setFont( fn, TRUE );
}

void QPEApplication::systemMessage( const QCString &msg, const QByteArray &data)
{
#ifdef Q_WS_QWS
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "linkChanged(QString)" ) {
	QString lf;
	stream >> lf;
	emit linkChanged( lf );
    } else if ( msg == "applyStyle()" ) {
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
	int r;
	stream >> r;
	if ( type() == GuiServer )
	    setDefaultRotation(r);
# if QT_VERSION >= 234
	int t = 0;
	switch ( r ) {
	    case 90:
		t = 1;
		break;
	    case 180:
		t = 2;
		break;
	    case 270:
		t = 3;
		break;
	}
	QWSDisplay::setTransformation( t );
# endif
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
    } else if ( msg =="categoriesChanged()" ) {
	emit categoriesChanged();
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
	::DateFormat tmp;
	stream >> tmp;
	emit dateFormatChanged( tmp );
    } else if ( msg == "setVolume(int,int)" ) {
	if ( type() == GuiServer ) {
	    int t,v;
	    stream >> t >> v;
	    setVolume(t,v);
	}
	emit volumeChanged( qtopia_muted );
    } else if ( msg == "volumeChange(bool)" ) {
	stream >> qtopia_muted;
	if ( type() == GuiServer ) {
	    setVolume();
	}
	emit volumeChanged( qtopia_muted );
    } else if ( msg == "flush()" ) {
	emit flush();
	// we need to tell the desktop
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "flushDone(QString)" );
	e << d->appName;
#endif
    } else if ( msg == "reload()" ) {
	// Reload anything stored in files...
	applyStyle();
	if ( type() == GuiServer ) {
	    setVolume();
	    setBacklight(-1);
	}
	// App-specifics...
	emit reload();
    } else if ( msg == "setScreenSaverMode(int)" ) {
	if ( type() == GuiServer ) {
	    int old = disable_suspend;
	    stream >> disable_suspend;
	    //qDebug("setScreenSaverMode(%d)", disable_suspend );
	    if ( disable_suspend > old )
		setScreenSaverInterval( -1 );
	}
    } else if ( msg == "getMarkedText()" ) {
	if ( type() == GuiServer ) {
	    const ushort unicode = 'C'-'@';
	    const int scan = Key_C;
	    qwsServer->processKeyEvent( unicode, scan, ControlButton, TRUE, FALSE );
	    qwsServer->processKeyEvent( unicode, scan, ControlButton, FALSE, FALSE );
	}
    } else if ( msg == "newChannel(QString)") {
	QString myChannel = "QPE/Application/" + d->appName;
	QString channel;
	stream >> channel;
	if (channel == myChannel) {
	    processQCopFile();
	    d->sendQCopQ();
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
#ifndef QT_NO_COP
	    QCopEnvelope e("QPE/System", "fastAppShowing(QString)");
	    e << d->appName;
#endif
	}

	d->show_mx(top,d->nomaximize, d->appName);
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
#ifndef QT_NO_COP
	    QCopEnvelope e("QPE/System", "fastAppShowing(QString)");
	    e << d->appName;
#endif
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
#ifndef QT_NO_COP
	QCopEnvelope e("QPE/System", "appRaised(QString)");
	e << d->appName;
#endif
    } else if ( msg == "flush()" ) {
	emit flush();
	// we need to tell the desktop
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "flushDone(QString)" );
	e << d->appName;
#endif
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
    } else if ( msg == "QPEProcessQCop()") {
	processQCopFile();
	d->sendQCopQ();
    } else {
	bool p = d->keep_running;
	d->keep_running = FALSE;
	emit appMessage( msg, data);
	if ( d->keep_running ) {
	    d->notbusysent = FALSE;
	    raiseAppropriateWindow();
	    if ( !p ) {
		// Tell the system we're still chugging along...
#ifndef QT_NO_COP
		QCopEnvelope e("QPE/System", "appRaised(QString)");
		e << d->appName;
#endif
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
#ifdef Q_WS_QWS
    if ( mw && argc() == 2 )
	Global::setDocument( mw, QString::fromUtf8(argv()[1]) );
#endif

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
#ifdef Q_WS_QWS
    if ( style == d->styleName )
	return;

    QStyle *newStyle = 0;

#if QT_VERSION >= 0x030000
    if ( style == "QPE"  || style == "Qtopia" ) {
	newStyle = new QPEStyle;
    } else {
	newStyle = QStyleFactory::create(style);
    }
#else
    StyleInterface *oldIface = d->styleIface;
    d->styleIface = 0;

    if ( style == "Windows" ) { // No tr
	newStyle = new QWindowsStyle;
    } else if ( style == "QPE" || style == "Qtopia" ) {
	newStyle = new QPEStyle;
    } else {
	if ( !d->styleLoader.inSafeMode() && d->styleLoader.isEnabled( style ) ) {
	    StyleInterface *iface = 0;
	    if ( d->styleLoader.queryInterface( style, IID_Style, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
		newStyle = iface->style();
		d->styleIface = iface;
	    }
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

#if QT_VERSION < 0x030000
    // cleanup old plugin.
    if ( oldIface )
	d->styleLoader.releaseInterface( oldIface );
#endif
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

	QPixmap pix;

#ifdef Q_WS_QWS
	pix = Resource::loadPixmap("bigwait");
#else
	//### revise add a different pix map
	qDebug("Missing pixmap : QPEApplication::prepareForTermination()");
#endif
	QLabel *lblWait = new QLabel(0, "wait hack!", QWidget::WStyle_Customize | // No tr
		QWidget::WStyle_NoBorder | QWidget::WStyle_Tool |
		QWidget::WStyle_StaysOnTop | QWidget::WDestructiveClose );
	lblWait->setPixmap( pix );
	lblWait->setAlignment( QWidget::AlignCenter );
	lblWait->setGeometry( desktop()->geometry() );
	lblWait->show();
	QTimer::singleShot( 5000, lblWait, SLOT(close()) ); // If we don't restart we want to get this out of the way
    }
#ifndef SINGLE_APP
#ifndef QT_NO_COP
    { QCopEnvelope envelope("QPE/System", "forceQuit()"); }
#endif
    processEvents(); // ensure the message goes out.
#ifndef Q_OS_WIN32
    sleep(1); // You have 1 second to comply.
#else
    Sleep(1000);
#endif
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
    } else {
	stylusDict->insert(w,(void*)mode);
	connect(w,SIGNAL(destroyed()),qApp,SLOT(removeSenderFromStylusDict()));
    }
}


/*!
  \reimp
*/
bool QPEApplication::eventFilter( QObject *o, QEvent *e )
{
    if ( !o->isWidgetType() )
	return FALSE;

    if ( stylusDict && e->type() >= QEvent::MouseButtonPress && e->type() <= QEvent::MouseMove ) {
	QMouseEvent* me = (QMouseEvent*)e;
	StylusMode mode = (StylusMode)(int)stylusDict->find(o);
	switch (mode) {
	  case RightOnHold:
	    switch ( me->type() ) {
	      case QEvent::MouseButtonPress:
		if ( me->button() == LeftButton ) {
		    if ( !d->presstimer )
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
    d->execCalled = TRUE;
#ifndef QT_NO_COP
    d->sendQCopQ();
    if ( !d->keep_running )
	processEvents(); // we may have received QCop messages in the meantime.
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
    if ( d->keep_running )
	d->store_widget_rect(d->qpe_main_widget, d->appName);

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
    if ( d->keep_running )
	d->store_widget_rect(d->qpe_main_widget, d->appName);

    processEvents();

    // If we are a preloaded application we don't actually quit, so emit
    // a System message indicating we're quasi-closing.
    if ( d->preloaded && d->qpe_main_widget ) {
#ifndef QT_NO_COP
	QCopEnvelope e("QPE/System", "fastAppHiding(QString)" );
	e << d->appName;
#endif
	d->qpe_main_widget->hide();
    } else {
	quit();
    }
}

#ifdef Q_WS_QWS
extern PluginLibraryManager *pluginLibraryManagerInstanceIntern();
#endif

void QPEApplication::pluginLibraryManager( PluginLibraryManager **m )
{
#ifdef Q_WS_QWS
    *m = pluginLibraryManagerInstanceIntern();
#else
    *m = 0;
#endif
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

#if (__GNUC__ > 2)
extern "C" void __cxa_pure_virtual();

void __cxa_pure_virtual()
{
    fprintf( stderr, "Pure virtual called\n"); // No tr
    abort();

}

#endif

#if defined(QPE_USE_MALLOC_FOR_NEW)

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
