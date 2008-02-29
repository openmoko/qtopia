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

#define QTOPIA_INTERNAL_LOADTRANSLATIONS
#define QTOPIA_INTERNAL_LANGLIST
#ifndef QTOPIA_INTERNAL_FILEOPERATIONS
#define QTOPIA_INTERNAL_FILEOPERATIONS
#endif


#include <qtopia/custom.h>

#include <stdlib.h>

#include <qfile.h>
#include <qlist.h>
#include <qqueue.h>
#include <qpainter.h>
#include <qvalidator.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#ifndef QTOPIA_DESKTOP
#include <qsoundqss_qws.h>
#endif

#ifdef Q_WS_QWS
#  ifndef QT_NO_COP
#    if QT_VERSION <= 231
#      define private public
#      define sendLocally processEvent
#      include <qtopia/qcopenvelope_qws.h>
#      undef private
#    else
#      include <qtopia/qcopenvelope_qws.h>
#    endif
#  endif
#  include <qwindowsystem_qws.h>
#endif

#include <qtextstream.h>
#include <qpalette.h>
#include <qwidgetlist.h>
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
#include <qmessagebox.h>
#include <qsignal.h>
#include <qclipboard.h>
#include <qtimer.h>
#include <qpixmapcache.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qaccel.h>
#include <qobjectlist.h>

#ifdef Q_WS_QWS
#define QTOPIA_INTERNAL_SENDINPUTHINT
#include <qwsdisplay_qws.h>
#endif

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#define QTOPIA_INTERNAL_INITAPP
#endif

#include <qtopia/qpeapplication.h>
#if QT_VERSION >= 0x030000
#  include <qstylefactory.h>
#else
#  include <qwindowsstyle.h>
#endif
#include <qtopia/global.h>
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
#  include <qutfcodec.h>
#endif
#include <qtopia/config.h>
#include <qtopia/storage.h>
#include <qtopia/network.h>
#include <qtopia/applnk.h>
#include <qtopia/resource.h>
#ifdef Q_WS_QWS
#  include <qtopia/fontmanager.h>
#  include <qtopia/fontdatabase.h>
#  include <qtopia/power.h>
#  include <qtopia/qpemenubar.h>
#  include <qtopia/imagecodecinterface.h>
#  include <qtopia/textcodecinterface.h>
#  include <qtopia/styleinterface.h>
#  ifdef QTOPIA_PHONE
#    include <qtopia/phonestyle.h>
#  else
#    include <qtopia/qpestyle.h>
#  endif
#  include <qtopia/private/qpedecoration_p.h>
#endif
#include <qtopia/alarmserver.h>
#include <qtopia/private/pluginloader_p.h>

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <sys/file.h>
#ifdef Q_WS_QWS
#include <errno.h>
#include <sys/stat.h>
#endif
#endif

#ifndef QTOPIA_DESKTOP
#ifndef Q_OS_WIN32
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#else
#include <mmsystem.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif // Q_OS_WIN32
#endif // QTOPIA_DESKTOP

#include "../qtopia1/qpe_show_dialog.cpp"

#if defined(QTOPIA_TEST)
#include <asm/timex.h>
#endif

#ifdef QTOPIA_PHONE
#include <qtopia/private/contextkeymanager_p.h>
#include <qtopia/contextmenu.h>

// from Resource
extern QStringList qpe_pathCache[Resource::AllResources + 1];
#endif

QIconSet qtopia_internal_loadIconSet( const QString &pix );

#ifdef QTOPIA_PHONE
extern bool qt_modalEditingEnabled;
bool mousePreferred = FALSE;
#endif

#ifdef QTOPIA_DESKTOP
#include <qdconfig.h>
#endif

QString qtopia_internal_homeDirPath();
QString qtopia_internal_defaultButtonsFile();

#ifdef QTOPIA_PHONE
const QArray<int> qpe_systemButtons()
{
    static QArray<int> *buttons = 0;

    if (!buttons) {
	buttons = new QArray<int>;
	Config cfg(qtopia_internal_defaultButtonsFile(), Config::File);
	cfg.setGroup("SystemButtons");
	int count = cfg.readNumEntry("Count", 0);
	if (count > 0) {
	    buttons->resize(count);
	    for (int i = 0; i < count; i++) {
		QString is = QString::number(i);
		(*buttons)[i] = QAccel::stringToKey(cfg.readEntry("Key"+is));
	    }
	}
    }

    return *buttons;
}

enum {
    MenuLikeDialog = 0x01,
} QPEWidgetFlags;

QMap<const QWidget *, int> qpeWidgetFlags;

#endif

#if defined(QTOPIA_TEST)
static cycles_t cycl_start, cycl_stop;
#endif


class HackWidget : public QWidget
{
public:
    bool needsOk()
    {
#ifndef QTOPIA_DESKTOP
	return (getWState() & WState_Reserved1 );
#else
	return FALSE;
#endif
    }

    QRect normalGeometry()
    { return topData()->normalGeometry; };
};

#ifdef Q_WS_QWS
extern Q_EXPORT QRect qt_maxWindowRect;
#endif

static const int npressticks=10;

class QPEApplicationData {
public:
    QPEApplicationData() : presstimer(0), presswidget(0), rightpressed(FALSE),
	kbgrabber(0), kbregrab(FALSE), notbusysent(FALSE), preloaded(FALSE),
	forceshow(FALSE), nomaximize(FALSE), qpe_main_widget(0),
	keep_running(TRUE), skiptimechanged(0), qcopQok(FALSE), hide_window(FALSE)
#ifdef Q_WS_QWS
	,styleLoader("styles"), styleIface(0), textPluginLoader("textcodecs"), // No tr
	imagePluginLoader("imagecodecs")
#endif
#ifdef QTOPIA_PHONE
	, editMenu(0)
#endif
    {
    }

    void drawPressTick()
    {
	if ( presstick < npressticks-2 ) {
	    QPainter p(presswidget);
	    Config cfg("presstick");
	    cfg.setGroup("PressTick");
	    QPixmap pm = Resource::loadPixmap(cfg.readEntry("Image"));
	    cfg.setGroup("Tick"+QString::number(presstick));
	    int dx = cfg.readNumEntry("Dx");
	    int dy = cfg.readNumEntry("Dy");
	    p.drawPixmap(presspos.x()+dx,presspos.y()+dy,pm);
	    pressdirty |= QRect(presspos.x()+dx,presspos.y()+dy,pm.width(),pm.height());
	}
    }
    void cancelPressAndHold()
    {
	if ( !pressdirty.isEmpty() ) {
	    presswidget->repaint(pressdirty);
	    pressdirty = QRect();
	}
	qApp->killTimer( presstimer );
	presstimer = 0;
    }

    int presstimer;
    QWidget* presswidget;
    QPoint presspos;
    int presstick;
    QRect pressdirty;
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
    QGuardedPtr<QWidget> lastraised;
    bool keep_running;
    int skiptimechanged;
    bool qcopQok;
    bool hide_window;    
    QQueue<QCopRec> qcopq;

    void enqueueQCop(const QCString &ch, const QCString &msg,
                               const QByteArray &data)
    {
	qcopq.enqueue(new QCopRec(ch,msg,data));
    }
    void sendQCopQ()
    {
	if ( qcopQok ) {
	    QCopRec* r;
	    while((r=qcopq.dequeue())) {
		// remove from queue before sending... 
		// event loop can come around again before getting
		// back from sendLocally
#ifndef QT_NO_COP
		QCopChannel::sendLocally(r->channel,r->message,r->data);
#endif
		delete r;
	    }
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
        if (hide_window)
            return;

	setWidgetCaptionFromAppName( mw, appName, QPEApplication::qpeDir() + "apps" );
	nomaximize = nomax;
	qpe_main_widget = mw;
	qcopQok = TRUE;
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
    QString decorationTheme;
#ifdef QTOPIA_PHONE
    QGuardedPtr<ContextMenu> editMenu;
    QStringList iconPath;
#endif
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
		if( img.isNull() )
		    img = qtopia_internal_loadIconSet(name).pixmap( QIconSet::Small, TRUE ).convertToImage();
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
	    if ( percent < 0 ) {
		percent = cfg.readNumEntry("Volume",50);
		qtopia_muted = cfg.readBoolEntry("Muted",false);
            }
#ifdef QTOPIA_USE_QSS_VOLUME
            QWSSoundClient sc;
            sc.setSystemVolume(percent);
#else
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
    // Add define for MAX_BRIGHTNESS to custom header to override setting
#ifdef MAX_BRIGHTNESS
    return MAX_BRIGHTNESS
#else
    if ( curbl == -1 ) {
	// Read from config
	Config config( "qpe" );
	config.setGroup( "Screensaver" );
	curbl = config.readNumEntry("Brightness",255);
    }
#endif

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
#ifdef QTOPIA_PHONE
static int homescreen_level = -1;
#endif

#ifdef Q_WS_QWS
static int disable_suspend = 100;

#ifndef QTOPIA_PHONE
static bool powerOrNetworkStatusChanged = FALSE;
static bool powerOnlineStopsSuspend()
{
    return !poweredsuspend_on && PowerStatusManager::readStatus().acStatus() == PowerStatus::Online;
}

static bool networkOnlineStopsSuspend()
{
    return !networkedsuspend_on && Network::networkOnline();
}
#endif

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
#ifdef QTOPIA_PHONE
        if (disable_suspend > 2 && homescreen_level != -1 && homescreen_level <= level) {
            if (homescreen_level == level) {
#ifndef QT_NO_COP
                QCopEnvelope showHome( "QPE/System", "showHomeScreen()" );
                return TRUE;
#endif      
            } else
                --level; 
        }
#endif
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
#ifndef QTOPIA_PHONE
            if (powerOnlineStopsSuspend() || networkOnlineStopsSuspend()) 
                powerOrNetworkStatusChanged = TRUE;
	    if ( disable_suspend > 2 && !powerOnlineStopsSuspend() && !networkOnlineStopsSuspend() )
	    {   
                if (PowerStatusManager::APMEnabled()) {
    		    QWSServer::processKeyEvent( 0xffff, Qt::Key_F34, FALSE, TRUE, FALSE );
		    return FALSE;
                } else {
                    if (powerOrNetworkStatusChanged) //restart screensaver
                    {
                        QWSServer::screenSaverActivate( FALSE );
                        powerOrNetworkStatusChanged = FALSE;
                        return FALSE;
                    }
                    return TRUE;
                }
            }
#else
            //no need to suspend in Phone Edition -> do nothing 
	    return TRUE;
#endif
	    break;
	}
	return FALSE;
    }
    static const int eventBlockLevel = -1;
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

    int v[5];
    i1 = ssi(i1, config, "Dim","Interval_Dim", 30); // No tr
    i2 = ssi(i2, config, "LightOff","Interval_LightOff", 20);
#ifndef QTOPIA_PHONE
    i3 = ssi(i3, config, "","Interval", 60); // No tr
#else
     // phone edition doesn't have 3rd screensaver level
#endif

    //qDebug("screen saver intervals: %d %d %d", i1, i2, i3);

    v[0] = QMAX( 1000*i1, 100);
    v[1] = QMAX( 1000*i2, 100);
#ifndef QTOPIA_PHONE
    v[2] = QMAX( 1000*i3, 100);
#else
    v[2] = 0;
#endif
    v[3] = 0;
    v[4] = 0;
   
#ifdef QTOPIA_PHONE
    config.setGroup( "HomeScreen" );
    bool showhomescreen_on = config.readBoolEntry("ShowHomeScreen", FALSE);
    if (showhomescreen_on) {
        int showhomescreen_interval;
        showhomescreen_interval = 1000*config.readNumEntry("Interval_HomeScreen", 30);
        for (int level=0; level < 5; level++) {
            if (v[level] && v[level] < showhomescreen_interval)
                showhomescreen_interval -= v[level];
            else if (!v[level]) {
                v[level]=showhomescreen_interval;
                homescreen_level = level;
                break;  
            } else {
                for (int i = 3; i >= level; i--) {
                    v[i+1] = v[i];
                }
                v[level] = showhomescreen_interval;
                v[level+1] = QMAX(v[level+1]-showhomescreen_interval, 100);
                homescreen_level = level;
                break;
            }
        }
        //qDebug(QString("%1 %2 %3 %4").arg(v[0]).arg(v[1]).arg(v[2]).arg(v[3]));
    } else
        homescreen_level = -1;

#endif
    dim_on = ( (i1 != 0) ? config.readNumEntry("Dim",1) : FALSE );
    lightoff_on = ( (i2 != 0 ) ? config.readNumEntry("LightOff",1) : FALSE );
    poweredsuspend_on = config.readNumEntry("Suspend",0);
    networkedsuspend_on = config.readNumEntry("NetworkedSuspend",1);
#ifdef Q_WS_QWS
    if ( !i1 && !i2 && !i3 
#ifdef QTOPIA_PHONE
            && !showhomescreen_on
#endif
        ){
	QWSServer::setScreenSaverInterval(0);
    }else{
#if QT_VERSION > 237
	QWSServer::setScreenSaverIntervals(v, QPEScreenSaver::eventBlockLevel);
#else
	QWSServer::setScreenSaverIntervals(v);
#endif
    }
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

#ifndef QT_NO_TRANSLATION
#include "global_qtopiapaths.cpp"
void QPEApplication::loadTranslations( const QStringList& qms )
{
    QStringList langs = Global::languageList();

    QStringList qpepaths = global_qtopiapaths();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
	for (QStringList::ConstIterator it = langs.begin(); it!=langs.end(); ++it) {
	    QString lang = *it;

	    QTranslator * trans;
	    QString tfn;

	    for (QStringList::ConstIterator qmit = qms.begin(); qmit!=qms.end(); ++qmit) {
		trans = new QTranslator(qApp);
		tfn = *qit + "i18n/" + lang + "/" + *qmit + ".qm";
		if ( trans->load( tfn ))
		    qApp->installTranslator( trans );
		else
		    delete trans;
	    }
	}
    }
}
#endif

static void qtopiaMsgHandler(QtMsgType type, const char* msg)
{
    switch ( type ) {
	case QtDebugMsg:
#ifdef DEBUG
	    fprintf( stderr, "Debug: %s\n", msg );
#endif
	    break;
	case QtWarningMsg:
	    fprintf( stderr, "Warning: %s\n", msg );
	    break;
	case QtFatalMsg:
	    fprintf( stderr, "Fatal: %s\n", msg );
	    abort();
    }
}

#if defined (_WS_QWS_)
extern bool qt_lineedit_password_visible_on_focus;
#endif

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
#if defined(QTOPIA_TEST)
, hasPerfMonitor(0)
#endif 
{
#if defined (_WS_QWS_)
    // pdas and phones have uncertain input, so as long as line edit
    // has focus, should be able to see password text.
    qt_lineedit_password_visible_on_focus = TRUE;
#endif
  
#if defined(Q_WS_QWS) && defined(Q_OS_UNIX)
    QString dataDir(Global::tempDir()); 
    if ( mkdir( dataDir.latin1(), 0700 ) ) {
	if ( errno != EEXIST ) {
	    qFatal( QString("Cannot create Qtopia data directory: %1")
		    .arg( dataDir ) );
	}
    }

    struct stat buf;
    if ( lstat( dataDir.latin1(), &buf ) )
	qFatal( QString( "stat failed for Qtopia data directory: %1" )
		.arg( dataDir ) );

    if ( !S_ISDIR( buf.st_mode ) )
	qFatal( QString( "%1 is not a directory" ).arg( dataDir ) );

    if ( buf.st_uid != getuid() )
	qFatal( QString( "Qtopia data directory is not owned by user %1: %2" )
		.arg( getuid() ).arg( dataDir ) );

    if ( (buf.st_mode & 0677) != 0600 )
	qFatal( QString( "Qtopia data directory has incorrect permissions: %1" )
		.arg( dataDir ) );
#endif
    qInstallMsgHandler(qtopiaMsgHandler);

#ifdef QTOPIA_PHONE
    Config config( qtopia_internal_defaultButtonsFile(), Config::File );
    config.setGroup( "Device" );
    QString pi = config.readEntry( "PrimaryInput", "Keypad" ).lower();
    // anything other than touchscreen means keypad modal editing gets enabled
    qt_modalEditingEnabled = pi != "touchscreen";
    mousePreferred = !qt_modalEditingEnabled;
    qDebug( "Modal editing is %s", qt_modalEditingEnabled ? "ENABLED" : "DISABLED" );
#endif

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
# ifdef QTOPIA_PHONE
	AppLnk::setSmallIconSize(16);
# else
	AppLnk::setSmallIconSize(10);
# endif
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

#ifdef QTOPIA_PHONE
    setFont( QFont( "helvetica", 14, QFont::Normal ) );
#endif

    QMimeSourceFactory::setDefaultFactory(new ResourceMimeFactory);

    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(hideOrQuit()));

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)

    sysChannel = new QCopChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(const QCString&,const QByteArray&)),
	     this, SLOT(systemMessage(const QCString&,const QByteArray&)) );
    
#if 0
    QCString channel = QCString(argv[0]);
    channel.replace(QRegExp(".*/"),"");
    d->appName = channel;
    channel = "QPE/Application/" + channel;
    pidChannel = new QCopChannel( channel, this);
    connect( pidChannel, SIGNAL(received(const QCString&,const QByteArray&)),
	    this, SLOT(pidMessage(const QCString&,const QByteArray&)));

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
    qms << "qt";
    qms << "libqpe";
    qms << "libqtopia";
    qms << "libqtopia2";
    qms << d->appName;

    loadTranslations(qms);
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
    loadTranslations(QStringList()<<channel);

    qt_fbdpy->setIdentity( channel ); // In Qt/E 2.3.6

    channel = "QPE/Application/" + channel;
    pidChannel = new QCopChannel( channel, this);
    connect( pidChannel, SIGNAL(received(const QCString&,const QByteArray&)),
	    this, SLOT(pidMessage(const QCString&,const QByteArray&)));

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

#ifdef QPE_LAZY_APPLICATION_SHUTDOWN
    if (!d->preloaded) {
	d->preloaded = TRUE;
	d->forceshow = TRUE;
    }
#endif

    /* overide stored arguments */
    setArgs(argc, argv);
}
#endif

struct InputMethodHintRec {
    InputMethodHintRec(QPEApplication::InputMethodHint h, const QString& p) :
	hint(h), param(p) {}
    QPEApplication::InputMethodHint hint;
    QString param;
};
static QPtrDict<InputMethodHintRec>* inputMethodDict=0;
static void createInputMethodDict()
{
    if ( !inputMethodDict ) {
	inputMethodDict = new QPtrDict<InputMethodHintRec>;
	inputMethodDict->setAutoDelete(TRUE);
    }
}

/*!
  Returns the currently set hint to the system as to whether
  widget \a w has any use for text input methods.

  \sa setInputMethodHint() InputMethodHint
*/
QPEApplication::InputMethodHint QPEApplication::inputMethodHint( QWidget* w )
{
    if ( inputMethodDict && w ) {
	InputMethodHintRec* r = inputMethodDict->find(w);
	if ( r ) return r->hint;
    }
    return Normal;
}

/*!
  Returns the currently set hint parameter for
  widget \a w.

  \sa setInputMethodHint() InputMethodHint
*/
QString QPEApplication::inputMethodHintParam( QWidget* w )
{
    if ( inputMethodDict && w ) {
	InputMethodHintRec* r = inputMethodDict->find(w);
	if ( r ) return r->param;
    }
    return QString::null;
}

/*!
    \enum QPEApplication::InputMethodHint

    \value Normal the widget sometimes needs text input.
    \value AlwaysOff the widget never needs text input.
    \value AlwaysOn the widget always needs text input.
    \value Number the widget needs numeric input.
    \value PhoneNumber the widget needs phone-style numeric input.
    \value Words the widget needs word input.
    \value Text the widget needs non-word input.
    \value Named the widget needs special input, defined by param.
	Each input method may support a different range of special
	input types, but will default to Text if they do not know the
	type.

    By default, QLineEdit and QMultiLineEdit have the Words hint
    unless they have a QIntValidator, in which case they have the Number hint.
    This is appropriate for most cases, including the input of names (new
    names being added to the user's dictionary).
    All other widgets default to Normal mode.

    \sa inputMethodHint() setInputMethodHint()
*/

/*!
  Hints to the system that widget \a w has use for the text input method
  specified by \a named. Such methods are input-method-specific and
  are defined by the files in $QPEDIR/etc/im/ for each input method.

  For example, the phone key input method includes support for the
  names input methods:

\list
  \i "email"
  \i "netmask"
  \i "url"
\endlist

  The effect in the phone key input method is to modify the binding of
  phone keys to characters (such as making "@" easier to input), and to
  add additional "words" to the recognition word lists (such as "www").

  If the current input method doesn't understand the hint, it will be
  ignored.

  \sa inputMethodHint() InputMethodHint
*/
void QPEApplication::setInputMethodHint( QWidget *w, const QString& named )
{
    setInputMethodHint(w,Named,named);
}

/*!
  Hints to the system that widget \a w has use for text input methods
  as specified by \a mode.

  \sa inputMethodHint() InputMethodHint
*/
void QPEApplication::setInputMethodHint( QWidget* w, InputMethodHint mode, const QString& param )
{
    createInputMethodDict();
    if ( mode == Normal ) {
	inputMethodDict->remove(w);
    } else {
	InputMethodHintRec *r = inputMethodDict->find(w);
	if ( r ) {
	    r->hint = mode;
	    r->param = param;
	} else {
	    r = new InputMethodHintRec(mode,param);
	    inputMethodDict->replace(w,r);
	}
    }
#ifdef QTOPIA_INTERNAL_SENDINPUTHINT
    if ( w->hasFocus() )
	sendInputHintFor(w,QEvent::None);
#endif
}

class HackDialog : public QDialog
{
public:
    void acceptIt() {
	qDebug("hack accept");
	accept(); }
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
#else
    Q_UNUSED( ke );
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
#ifndef QTOPIA_PHONE
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
#ifdef QTOPIA_PHONE
			if (isMenuLike(d))
			    d->rejectIt();
			else
#endif
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
#else
# ifdef QT_QWS_SL5XXX
	if ( ke->simpleData.keycode == Qt::Key_F33 )
	    ke->simpleData.keycode = Qt::Key_Back;
	else if (ke->simpleData.keycode == Qt::Key_F30)
	    ke->simpleData.keycode = Qt::Key_Select;
	else if (ke->simpleData.keycode == Qt::Key_Escape)
	    ke->simpleData.keycode = Qt::Key_Back;
# endif

	if (ke->simpleData.keycode == Qt::Key_Back ||
	    ke->simpleData.keycode == Qt::Key_No) {
	    QWidget *active = activeWindow();
	    QWidget *popup = 0;
	    if ( active && active->isPopup() ) {
		popup = active;
		active = active->parentWidget();
	    } else if (active) {
		// If the focus widget (or parent) requires the context key
		// then we can't close the dialog.
		QWidget *focus = active->focusWidget();
		if (focus) {
		    QWidget *tw =
			ContextKeyManager::instance()
			    ->findTargetWidget(focus, ke->simpleData.keycode,
				(mousePreferred || focus->isModalEditing())
				);
		    if ( tw && tw != active )
			active = 0;
		}
	    }

	    if ( active && (int)active->winId() == ke->simpleData.window &&
		 !active->testWFlags( WStyle_Customize|WType_Popup|WType_Desktop )) {
		if ( ke->simpleData.is_press ) {
		    if ( popup )
			popup->close();
		    bool qt_singleFocusWidget(const QWidget *in); // qwidget.cpp
		    if ( active->inherits( "QDialog" )
			&& (mousePreferred || (!active->editingWidget()
			    || qt_singleFocusWidget(active->editingWidget())))
			&& ke->simpleData.keycode != Qt::Key_No
			&& active->testWFlags(WType_Modal))
		    {
			HackDialog *d = (HackDialog *)active;
			if (isMenuLike(d)) {
			    d->rejectIt();
			} else {
			    d->acceptIt();
			}
			return TRUE;
		    }
		}
	    }
	}
#endif

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
	    if ( topm && (int)topm->winId() != fe->simpleData.window) {
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
#ifdef QTOPIA_PHONE
    delete d->editMenu;
#endif
    delete d;
}

/*!
  Returns \c{$QPEDIR/}.
*/
QString QPEApplication::qpeDir()
{
#ifdef QTOPIA_DESKTOP
    return gQtopiaDesktopConfig->installedDir();
#endif

#ifdef Q_WS_QWS
    return *global_qtopiapaths().begin();
#endif // Q_WS_QWS
}

/*!
  Returns the user's current Document directory. There is a trailing "/".
*/
QString QPEApplication::documentDir()
{
    QString r = ::qtopia_internal_homeDirPath();

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

#ifdef Q_WS_QWS
extern int qws_display_id;
#endif

/*!
  Rotates Qtopia by \a r degrees. 
*/
void QPEApplication::setDefaultRotation(int r)
{
    if ( qApp->type() == GuiServer ) {
	deforient = r;
#if defined(Q_WS_QWS) && !defined(Q_OS_WIN32)
	setenv("QWS_DISPLAY", QString("Transformed:Rot%1:%2").arg(r).arg(qws_display_id).latin1(), 1);
#endif
	Config config("qpe");
        config.setGroup( "Rotation" );
	config.writeEntry( "Screen", getenv("QWS_DISPLAY") );

    } else {
#ifndef QT_NO_COP
	QCopEnvelope("QPE/System", "setDefaultRotation(int)") << r;
#endif
    }
}


#define setPaletteEntry(pal, cfg, role, defaultVal) \
    setPalEntry(pal, cfg, #role, QColorGroup::role, defaultVal)
static void setPalEntry( QPalette &pal, const Config &config, const QString &entry,
				QColorGroup::ColorRole role, const QString &defaultVal )
{
    QString value =
	((Config*)&config)-> // FIX in Qtopia 4 (no need to cast)
	readEntry( entry, defaultVal );
    if ( value[0] == '#' )
	pal.setColor( role, QColor(value) );
    else {
	QPixmap pix;
#ifdef SIZE_WALLPAPER_TO_DESKTOP_SIZE  // Disabled at the moment
	QSize siz = qApp->desktop()->size();
	int wid = siz.width(), h = siz.height();
	pix = Resource::loadImage(value).smoothScale(wid,h);
#endif
	pix = Resource::loadPixmap(value);
	pal.setBrush( role, QBrush(QColor(defaultVal), pix) );
    }
}


/*!
  \internal
*/
void QPEApplication::applyStyle()
{
#ifdef Q_WS_QWS
    QString styleName;
    Config config( "qpe" );

    config.setGroup( "Appearance" );

#ifdef QTOPIA_PHONE
    QString theme = config.readEntry("Theme", "qtopia.conf");
    QString themeDir = QPEApplication::qpeDir() + "etc/themes/";
    theme = theme.replace(QRegExp("\\.desktop"), ".conf"); // backwards compat

    if (theme.isEmpty() || !QFile::exists(themeDir + theme))
	theme = "qtopia.conf";

    Config themeCfg(themeDir + theme, Config::File);
    themeCfg.setGroup("Theme");

    // Update the icon path
    bool iconsChanged = FALSE;
    QStringList extraPath = qpe_pathCache[Resource::PixResource];
    if (d->iconPath.count()) {
	// remove paths that the last theme set
	QStringList::Iterator it;
	for (it = d->iconPath.begin(); it != d->iconPath.end(); ++it) {
	    extraPath.remove(QPEApplication::qpeDir() + *it);
	    extraPath.remove(*it);
	}
	iconsChanged = TRUE;
    }

    d->iconPath = themeCfg.readListEntry("IconPath", ';');
    if (d->iconPath.count())
	iconsChanged = TRUE;
    extraPath += d->iconPath;
    qpe_pathCache[Resource::PixResource].clear();
    for (QStringList::ConstIterator prefix = extraPath.begin(); prefix!=extraPath.end(); ++prefix) {
	QString resourcePath = QPEApplication::qpeDir() + *prefix;
	if (QFile::exists(resourcePath)) {
	    qpe_pathCache[Resource::PixResource].append(resourcePath);
	} else {
	    if (QFile::exists( *prefix ))
		qpe_pathCache[Resource::PixResource].append( *prefix );
	}
    }

    if (iconsChanged)
	QPixmapCache::clear();

    styleName = config.readEntry( "Style", "phonestyle" );
#else
    styleName = config.readEntry( "Style", "Qtopia" );
#endif

    // Widget style
    internalSetStyle( styleName );

    // Colors
    // This first bit is same as settings/appearence/appearence.cpp, readColorScheme()
    QPalette tempPal;
    setPaletteEntry( tempPal, config, Button, "#F0F0F0" );
    setPaletteEntry( tempPal, config, Background, "#EEEEEE" );
    QPalette pal( tempPal.normal().button(), tempPal.normal().background() );
    setPaletteEntry( pal, config, Button, "#F0F0F0" );
    setPaletteEntry( pal, config, Background, "#EEEEEE" );
    setPaletteEntry( pal, config, Base, "#FFFFFF" );
    setPaletteEntry( pal, config, Highlight, "#8BAF31" );
    setPaletteEntry( pal, config, Foreground, "#000000" );
    QString color = config.readEntry( "HighlightedText", "#FFFFFF" );
    pal.setColor( QColorGroup::HighlightedText, QColor(color) );
    color = config.readEntry( "Text", "#000000" );
    pal.setColor( QColorGroup::Text, QColor(color) );
    color = config.readEntry( "ButtonText", "#000000" );
    pal.setColor( QPalette::Active, QColorGroup::ButtonText, QColor(color) );

    QString val = config.readEntry( "Shadow" );
    if (!val.isEmpty()) {
	pal.setColor( QColorGroup::Shadow, QColor(val));
    } else {
	pal.setColor( QColorGroup::Shadow,
	    pal.color(QPalette::Normal, QColorGroup::Button).dark(400) );
    }

    val = config.readEntry( "Text_disabled" );
    if (!val.isEmpty()) {
	pal.setColor( QPalette::Disabled, QColorGroup::Text, QColor(val));
    } else {
	pal.setColor( QPalette::Disabled, QColorGroup::Text,
	    pal.color(QPalette::Active, QColorGroup::Base).dark() );
    }

    val = config.readEntry( "Foreground_disabled" );
    if (!val.isEmpty()) {
	pal.setColor( QPalette::Disabled, QColorGroup::Foreground, QColor(val));
    } else {
	pal.setColor( QPalette::Disabled, QColorGroup::Foreground,
	    pal.color(QPalette::Active, QColorGroup::Background).dark() );
    }

    color = config.readEntry( "AlternateBase", "#EEF5D6" );
    if ( color[0] == '#' )
	style().setExtendedBrush(QStyle::AlternateBase, QColor(color));
    else {
	QPixmap pix;
#ifdef SIZE_WALLPAPER_TO_DESKTOP_SIZE  // Disabled at the moment
	QSize siz = qApp->desktop()->size();
	int wid = siz.width(), h = siz.height();
	pix = Resource::loadImage(color).smoothScale(wid,h);
#endif
	pix = Resource::loadPixmap(color);
	style().setExtendedBrush(QStyle::AlternateBase, QBrush(QColor("#EEF5D6"),pix));
    }

    setPalette( pal, TRUE );

    // Unpublished feature for now, since current windows will be broken.
    // Need to go through all windows and poke QScrollViews to updateScrollbars.
    extern bool qt_left_hand_scrollbars;
    int lh = config.readNumEntry( "LeftHand", 0 );
    qt_left_hand_scrollbars = lh;

    // Window Decoration
    QString dec = config.readEntry( "Decoration", "Qtopia" );
    QString decTheme = config.readEntry( "DecorationTheme", "" );
    if ( dec != d->decorationName || !decTheme.isEmpty()) {
	qwsSetDecoration( new QPEDecoration( dec ) );
	d->decorationName = dec;
	d->decorationTheme = decTheme;
    }

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

#ifdef QTOPIA_PHONE
    if ( styleName == "phonestyle" || styleName == "Qtopia" ) {
	QString path = themeCfg.readEntry("WidgetConfig");
	((PhoneStyle*)&style())->setTheme(!!path ? themeDir+path : path);
    }
#endif
#endif
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
    } else if ( msg == "close()" ) {
	if ( type() != GuiServer )
	    hideOrQuit();
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
	    l += ".utf8";
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
	// doesn't make sense to do this if quicklauncher,
	// and may steal call to something that needs it.
	if (d->appName != "quicklauncher") {
	    d->skiptimechanged++;
	    QString t;
	    stream >> t;
	    if ( t.isNull() )
		unsetenv( "TZ" );
	    else
		setenv( "TZ", t.latin1(), 1 );
	    // emit the signal so everyone else knows...
	    emit timeChanged();
	}
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
            {
            Config cfg("Sound");
            cfg.setGroup("System");
            cfg.writeEntry("Muted",qtopia_muted);
            }
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
	    stream >> disable_suspend;
	    qwsServer->restartScreenSaverTimer();
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
    } else if ( msg == "wordsChanged(QString,int)" ) {
	QString dictname;
	int pid;
	stream >> dictname >> pid;
	void qtopia_reload_words(const QString& dictname); // global.cpp
	if ( pid != getpid() ) {
	    qDebug("Reload words '%s' (I am %d, not %d)",
		dictname.latin1(),getpid(),pid);
	    qtopia_reload_words(dictname);
	}
    } else if ( msg == "RecoverMemory()" ) {
	if (qApp->type() != GuiServer) {
	    QPixmapCache::clear();
	}
    } 
#ifdef QTOPIA_PHONE
    else if ( msg == "updateContextBar()" ) {
	if (activeWindow() && focusWidget()) {
	    ContextKeyManager::instance()->updateContextLabels();
	}
    }
#endif
#else
    Q_UNUSED( msg );	
    Q_UNUSED( data );	
#endif
}

/*!
  \internal
*/
bool QPEApplication::raiseAppropriateWindow()
{
    bool r=FALSE;

    // 1. Raise the main widget
    QWidget *top = d->qpe_main_widget;
    if ( !top ) top = mainWidget();

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
    }

    QWidget *topm = activeModalWidget();

    // 2. Raise any parentless widgets (except top and topm, as they
    //     are raised before and after this loop).  Order from most
    //     recently raised as deepest to least recently as top, so
    //     that repeated calls cycle through widgets.
    QWidgetList *list = topLevelWidgets();
    if ( list ) {
	bool foundlast = FALSE;
	QWidget* topsub = 0;
	if ( d->lastraised ) {
	    for (QWidget* w = list->first(); w; w = list->next()) {
		if ( !w->parentWidget() && w != top && w != topm && w->isVisible() && !w->isDesktop() ) {
		    if ( w == d->lastraised )
			foundlast = TRUE;
		    if ( foundlast ) {
			w->raise();
			topsub = w;
		    }
		}
	    }
	}
	for (QWidget* w = list->first(); w; w = list->next()) {
	    if ( !w->parentWidget() && w != top && w != topm && w->isVisible() && !w->isDesktop() ) {
		if ( w == d->lastraised )
		    break;
		w->raise();
		topsub = w;
	    }
	}
	d->lastraised = topsub;
	delete list;
    }

    // 3. Raise the active modal widget.
    if ( topm && topm != top ) {
	topm->show();
	topm->raise();
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
	if (d->qpe_main_widget) {
	    d->preloaded = TRUE;
	    d->forceshow = FALSE;
	}
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
#else
    Q_UNUSED( msg );	
    Q_UNUSED( data );	
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
  \internal
*/
void QPEApplication::setHideWindow()
{
    if ( qApp && qApp->inherits( "QPEApplication" ) ) {
        QPEApplication *qpeApp = (QPEApplication*)qApp;
        qpeApp->d->hide_window = TRUE;
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
    if (style != "themedstyle"){
	if ( style == d->styleName)
	    return;
    }else{
	Config config( "qpe" );
	config.setGroup( "Appearance" );
	// For the Pixmap style we must remove the existing style here for theme changes to take effect
	setStyle( new QWindowsStyle );
#if QT_VERSION < 0x030000
	if ( d->styleIface )
	    d->styleLoader.releaseInterface( d->styleIface );
	d->styleIface = 0;
#endif
    }

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
    } else if ( style == "QPE" || style == "Qtopia" 
# ifdef QTOPIA_PHONE
	    || style == "phonestyle" ) {
	newStyle = new PhoneStyle;
# else
	) {
	newStyle = new QPEStyle;
# endif
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
#ifdef QTOPIA_PHONE
	newStyle = new PhoneStyle;
#else
	newStyle = new QPEStyle;
#endif
	d->styleName = "QPE";
    } else {
	d->styleName = style;
    }

    if (&qApp->style() != newStyle)
	setStyle( newStyle );


#if QT_VERSION < 0x030000
    // cleanup old plugin.
    if ( oldIface )
	d->styleLoader.releaseInterface( oldIface );
#endif
#else
    Q_UNUSED( style );
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
#ifndef QTOPIA_PHONE
	pix = Resource::loadPixmap("bigwait");
#else
	pix = Resource::loadPixmap("splash");
#endif
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
    Global::sleep(1); // In libqtopia2
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

static QPtrDict<QString>* imdict=0;

void QPEApplication::removeSenderFromIMDict()
{
#ifdef QTOPIA_PHONE
    imdict->remove((void*)sender());
#else
    Q_UNUSED(imdict);
#endif
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
		    d->presstick = npressticks;
		    if ( !d->presstimer )
			d->presstimer = startTimer(500/d->presstick); // #### pref.
		    d->presswidget = (QWidget*)o;
		    d->presspos = me->pos();
		    d->rightpressed = FALSE;
		}
		break;
	      case QEvent::MouseMove:
		if (d->presstimer && (me->pos()-d->presspos).manhattanLength() > 8)
		    d->cancelPressAndHold();
		break;
	      case QEvent::MouseButtonRelease:
		if ( me->button() == LeftButton ) {
		    if ( d->presstimer )
			d->cancelPressAndHold();
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
	if ( ke->key() == Key_Enter ) 
	{
	    if ( o->isA( "QRadioButton" ) || o->isA( "QCheckBox" ) ) 
	    {
		postEvent( o, new QKeyEvent( e->type(), Key_Space, ' ',
		    ke->state(), " ", ke->isAutoRepeat(), ke->count() ) );
		return TRUE;
	    }
	}
#ifdef QTOPIA_PHONE
	else if ( !mousePreferred && (ke->key() == Key_Left || ke->key() == Key_Right || ke->key() == Key_Up || ke->key() == Key_Down)) 
	{
	    if (o->inherits("QLineEdit")) 
	    {
		QLineEdit *l = (QLineEdit*)o;
		if ( l->isModalEditing() ) 
		{
		    if (l->text().length() == 0 || l->isReadOnly())
			ContextKeyManager::instance()->setStandard(Qt::Key_Back, ContextBar::Cancel);
		    else if (l->cursorPosition() == 0)
			ContextKeyManager::instance()->setStandard(Qt::Key_Back, ContextBar::NoLabel);
		    else
			ContextKeyManager::instance()->setStandard(Qt::Key_Back, ContextBar::BackSpace);
		}
	    } else if (o->inherits("QMultiLineEdit")) {
		QMultiLineEdit *l = (QMultiLineEdit*)o;
		if (  l->isModalEditing() ) 
		{
		    if (l->length() == 0 || l->isReadOnly()) 
		    {
			ContextKeyManager::instance()->setStandard(Qt::Key_Back, ContextBar::Cancel);
		    } 
		    else 
		    {
			int line, col;
			l->getCursorPosition(&line, &col);
			if (!line && !col)
			    ContextKeyManager::instance()->setStandard(Qt::Key_Back, ContextBar::NoLabel);
			else
			    ContextKeyManager::instance()->setStandard(Qt::Key_Back, ContextBar::BackSpace);
		    }
		}
	    }
	} // end else if cursor dir.
	else if (ke->key() == ContextMenu::key()) {
	    QWidget *w = (QWidget *)o;
	    QWidget *cw = ContextKeyManager::instance()->findTargetWidget(w, 
			ke->key(), mousePreferred || w->isModalEditing());
	    if (cw && cw != w) 
	    {
		qDebug("redirect to: %s from %s", cw->className(), w->className());
		sendEvent(cw,e);
		return TRUE;
	    }
	} // end elsif menu key
#endif  // end QTOPIA_PHONE
    } // end else if key event.
#if defined(QTOPIA_INTERNAL_SENDINPUTHINT) || defined(QTOPIA_PHONE)
    else if ( e->type() == QEvent::FocusIn 
#ifdef QTOPIA_PHONE
	      || (!mousePreferred && (e->type() == QEvent::EnterModalEdit ||
	      e->type() == QEvent::LeaveModalEdit)) 
#endif
	      ) {

	QWidget *w = (QWidget *)o;

	if (!activeWindow())
	    return FALSE;

	sendInputHintFor(w, e->type());
#ifdef QTOPIA_PHONE
	if ( (mousePreferred || w->isModalEditing()) && 
		(w->inherits("QLineEdit") || w->inherits("QMultiLineEdit"))) {
	    if (!ContextMenu::internalMenuFor(w, ContextMenu::Modal)) {
		if (!d->editMenu)
		    d->editMenu = ContextMenu::createEditMenu(0, ContextMenu::Modal);
		d->editMenu->addTo(w, ContextMenu::Modal);
	    }
	}
#endif
    }
#endif
#ifdef QTOPIA_PHONE
    else if ( e->type() == QEvent::FocusOut ) {
	QWidget *w = (QWidget *)o;
	if (!focusWidget() && w->topLevelWidget()->isVisible() && activeWindow()) {
	    ContextKeyManager::instance()->updateContextLabels();
	}
	if (d->editMenu && (w->inherits("QLineEdit") || w->inherits("QMultiLineEdit")))
	    d->editMenu->removeFrom(w);
    } else if (e->type() == QEvent::Show && ((QWidget*)o)->isModal()) {
	if (o->inherits("QMessageBox")) {
	    QMessageBox *mb = (QMessageBox*)o;
	    QPushButton *pb1 = (QPushButton*)mb->child("button1", "QPushButton"); // No tr
	    QPushButton *pb2 = (QPushButton*)mb->child("button2", "QPushButton"); // No tr
	    QPushButton *pb3 = (QPushButton*)mb->child("button3", "QPushButton"); // No tr
	    if (mb->icon() == QMessageBox::Information)
		mb->setIconPixmap(Resource::loadPixmap("alert_info"));
	    else if (mb->icon() == QMessageBox::Warning)
		mb->setIconPixmap(Resource::loadPixmap("alert_warning"));
	    if (!pb3 && pb1) {
		// We only handle MBs with 1 or 2 buttons.
		// Currently we assume that the affirmative is pb1 and
		// the negative is pb2.

		// Calculate how big dlg is and resize to fit screen nicely.
		QRect fg = mb->frameGeometry();
		QRect cg = mb->geometry();
		int maxX = desktop()->width() - (fg.width()-cg.width());
		QLabel *label = (QLabel*)mb->child("text", "QLabel"); // No tr
		int space = mb->width() - label->width();
		int width = maxX - space - 2;
		int reqHeight = QMAX(32,label->heightForWidth(width))+2*label->y();
		mb->setMinimumSize(width, reqHeight);
		QApplication::sendPostedEvents( this, QEvent::Resize );
		if (reqHeight < qt_maxWindowRect.height()*3/4) {
		    mb->setGeometry(cg.left()-fg.left(),
			    qt_maxWindowRect.bottom() - reqHeight + 1,
			    maxX, reqHeight);
		} else {
		    mb->showMaximized();
		}
		mb->setFocus();
		if (label)
		    label->resize(width, QMAX(label->font().pointSize()+4,label->heightForWidth(width)));
		// Setup accels for buttons and hide the buttons.
		pb1->setFocusPolicy(QWidget::NoFocus);
		pb1->setDefault(FALSE);
		int accel = 0;
		if (pb2) {
		    if (qpe_systemButtons().contains(Key_Yes)) {
			accel = Key_Yes;
		    } else {
			const QArray<int> &cbtns = ContextBar::keys();
			if (cbtns.count()) {
			    if (cbtns[0] != Key_Back)
				accel = cbtns[0];
			    else if (cbtns.count() > 1)
				accel = cbtns[cbtns.count()-1];
			}
		    }
		}
		if (accel) {
		    pb1->setAccel(accel);
		    ContextBar::setLabel(mb, accel, QString::null, pb1->text());
		}
		pb1->move(-1000, -1000);
		if (pb2) {
		    pb2->setFocusPolicy(QWidget::NoFocus);
		    pb2->setDefault(FALSE);
		    accel = qpe_systemButtons().contains(Key_No) ? Key_No : Key_Back;
		    pb2->setAccel(accel);
		    ContextBar::setLabel(mb, accel, QString::null, pb2->text());
		    pb2->move(-1000, -1000);
		}
	    }
	} else if (o->inherits("QDialog") && !qpe_systemButtons().contains(Key_No)) {
	    QDialog *dlg = (QDialog *)o;
	    if (!isMenuLike(dlg)) {
		if (!dlg->testWFlags(WStyle_Customize) || dlg->testWFlags(WStyle_SysMenu)) {
		    QObject *co = 0;
		    QObjectList *childObjects = (QObjectList*)o->children();
		    if (childObjects) {
			for (co = childObjects->first(); co!=0; co = childObjects->next()) {
			    if (co->isWidgetType() && co->isA("ContextMenu")) {
				break;
			    }
			}
		    }
		    if (!co) {
			// There is no context menu defined
			(void)new ContextMenu((QWidget*)o);
		    }
		}
	    }
	}
    }
#endif  // QTOPIA_PHONE

    return FALSE;
}

#ifdef QTOPIA_INTERNAL_SENDINPUTHINT
void QPEApplication::sendInputHintFor(QWidget *w, QEvent::Type etype)
{
#ifndef QTOPIA_PHONE
    Q_UNUSED(etype);
#endif
    const QValidator* v = 0;
    InputMethodHintRec *hr=0;
    if ( inputMethodDict ) {
	hr = inputMethodDict->find(w);
	if ( !hr ) {
	    QWidget* p = w->parentWidget();
	    if ( p ) {
		if ( p->focusProxy() == w ) {
		    hr = inputMethodDict->find(p);
		} else {
		    p = p->parentWidget();
		    if ( p && p->focusProxy() == w )
			hr = inputMethodDict->find(p);
		}
	    }
	}
    }
    int n = hr ? (int)hr->hint : 0;
    if ( w->inherits("QLineEdit") ) {
	QLineEdit* l = (QLineEdit*)w;
	if ( !n && !l->isReadOnly()) {
	    n = l->echoMode()==QLineEdit::Normal ? (int)Words : (int)Text;
	    v = l->validator();
	}
#ifdef QTOPIA_PHONE
	if( n && !mousePreferred ) {
	    if (etype == QEvent::EnterModalEdit) {
		connect(l, SIGNAL(textChanged(const QString&)),
			qApp, SLOT(lineEditTextChange(const QString&)));
	    } else if (etype == QEvent::LeaveModalEdit) {
		disconnect(l, SIGNAL(textChanged(const QString&)),
			qApp, SLOT(lineEditTextChange(const QString&)));
	    }
	}
#endif
    } else if ( w->inherits("QMultiLineEdit") ) {
	QMultiLineEdit* l = (QMultiLineEdit*)w;
	if ( !n && !l->isReadOnly()) {
	    n = (int)Words;
	    v = l->validator();
	}
#ifdef QTOPIA_PHONE
	if( n && !mousePreferred ) {
	    if (etype == QEvent::EnterModalEdit)
		connect(l, SIGNAL(textChanged()),
			qApp, SLOT(multiLineEditTextChange()));
	    else if (etype == QEvent::LeaveModalEdit)
		disconnect(l, SIGNAL(textChanged()),
			qApp, SLOT(multiLineEditTextChange()));
	}
#endif
    }
    if ( !hr && v && v->inherits("QIntValidator") )
	n = (int)Number;

    Q_INT32 windowId = 0;
    if (w) {
	// popups don't send window active events... so need
	// to find the window id for the actual active window,
	// rather than that of the popup.
	QWidget *p = w->topLevelWidget();;
	while(p->isPopup() && p->parentWidget())
	    p = p->parentWidget();
	windowId = p->topLevelWidget()->winId();
    }
    // find ancestor.. top ancestor, then get its' window id
    if ( n == Named ) {
	QCopEnvelope env("QPE/IME", "inputMethodHint(QString,int)" );
	env << (hr ? hr->param : QString::null);
	env << windowId;
    } else {
	QCopEnvelope env("QPE/IME", "inputMethodHint(int,int)" );
	env << n;
	env << windowId;
    }
#ifdef QTOPIA_PHONE
    if (qApp->focusWidget() == w && qApp->activeWindow()) {
	ContextKeyManager::instance()->updateContextLabels();
    }                    
#endif
}
#endif

#if defined(QTOPIA_PHONE) || defined(QTOPIA_TEST) // since not binary compatible
/*!
  \internal
*/
bool QPEApplication::notify(QObject* o, QEvent* e)
{
#if defined(QTOPIA_TEST)
    QString objectName;
    bool isMonitoring = FALSE; //racing condition
    if ( hasPerfMonitor ) {
        isMonitoring = TRUE;
        objectName = o->className(); // save class name in case o is deleted after processing event
        cycl_start = get_cycles(); // returns 0 if not available for cpu
    }
#endif
    bool r = QApplication::notify(o,e);
#if defined(QTOPIA_PHONE)
    if ( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease ) {
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( !ke->isAccepted() ) {
	    // Send some unaccepted keys back to the server for processing.
	    if ( ke->key() == Key_Hangup || ke->key() == Key_Call || ke->key() == Key_Flip ) {
		QCopEnvelope e("QPE/System","serverKey(int,int)");
		e << ke->key() << int(ke->type() == QEvent::KeyPress);
	    }
	}
    }
#endif
    
#if defined(QTOPIA_TEST) 
    if ( hasPerfMonitor && isMonitoring) {
        cycl_stop = get_cycles(); 
        ulong cycl = (ulong)(cycl_stop-cycl_start);
        app_slave.newCycleData(cycl, objectName, e->type()); 
    }
#endif
    return r;
}

#endif

/*!
  \reimp
*/
void QPEApplication::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == d->presstimer && d->presswidget ) {
	if ( d->presstick ) {
	    d->presstick--;
	    d->drawPressTick();
	} else {
	    // Right pressed
	    d->cancelPressAndHold();
	    postEvent( d->presswidget,
		new QMouseEvent( QEvent::MouseButtonPress, d->presspos,
			    RightButton, LeftButton ) );
	    d->rightpressed = TRUE;
	}
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

  Under Qtopia Phone Edition this function does nothing.  It is not
  possible to grab the keyboard under Qtopia Phone Edition.

  \sa ungrabKeyboard()
*/
void QPEApplication::grabKeyboard()
{
#ifndef QTOPIA_PHONE
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
#endif
}

/*!
  \reimp
*/
int QPEApplication::exec()
{
    d->qcopQok = TRUE;
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
        if ( !d->hide_window )
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

void QPEApplication::lineEditTextChange(const QString &
#ifdef QTOPIA_PHONE
s
#endif
)
{
#ifdef QTOPIA_PHONE
    if( !mousePreferred ) {
	QLineEdit *l = (QLineEdit*)sender();
	if (s.length() == 0 || l->isReadOnly())
	    ContextKeyManager::instance()->setStandard(Qt::Key_Back, ContextBar::Cancel);
	else
	    ContextKeyManager::instance()->setStandard(Qt::Key_Back, ContextBar::BackSpace);
    }
#endif
}

void QPEApplication::multiLineEditTextChange()
{
#ifdef QTOPIA_PHONE
    if( !mousePreferred ) {
	QMultiLineEdit *l = (QMultiLineEdit*)sender();
	if (l->length() == 0 || l->isReadOnly())
	    ContextKeyManager::instance()->setStandard(Qt::Key_Back, ContextBar::Cancel);
	else
	    ContextKeyManager::instance()->setStandard(Qt::Key_Back, ContextBar::BackSpace);
    }
#endif
}

void QPEApplication::removeFromWidgetFlags()
{
#ifdef QTOPIA_PHONE
    // removes calling object from flags.
    const QWidget *s = (const QWidget *)sender();
    if (qpeWidgetFlags.contains(s))
	qpeWidgetFlags.remove(s);
#endif
}

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
    if (p)
	free(p);
}

void operator delete[](void* p, size_t /*size*/)
{
    if (p)
	free(p);
}

void operator delete(void* p)
{
    if (p)
	free(p);
}

void operator delete(void* p, size_t /*size*/)
{
    if (p)
	free(p);
}

#endif

#ifdef QTOPIA_PHONE
#include "../qtopia2/qpeapplication_menulike.cpp"
#endif

#if defined QTOPIA_TEST

/*!
  \fn void QPEApplication::stopCycleCount()

  \internal
*/

/*!
  \fn void QPEApplication::startCycleCount()
  \internal
*/

/*!
  \internal
*/
QString QPEApplication::appName()
{
    if (d != 0) {
        return d->appName;
    } else {
        QFileInfo fi( QString(this->argv()[0]) );
        QString app_name = fi.baseName();
        int pos;
        do {
            pos = app_name.find("/");
            if (pos >= 0)
                app_name = app_name.mid(pos+1);
        } while (pos >= 0);

        if (app_name.isEmpty())
            return "ERROR_NO_APPNAME";
        else
            return app_name;
    }
}
#endif
