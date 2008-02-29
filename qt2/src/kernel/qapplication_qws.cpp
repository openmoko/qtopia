/****************************************************************************
** $Id: qt/src/kernel/qapplication_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of Qt/FB startup routines and event handling
**
** Created : 991025
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

// Started with from qapplication_x11.cpp,v 2.399 1999/10/22 14:39:33

#define select		_qt_hide_select
#define gettimeofday	_qt_hide_gettimeofday

#include "qglobal.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qbitarray.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qdatetime.h"
#include "qtextcodec.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qsocketnotifier.h"
#include "qsessionmanager.h"
#include "qvaluelist.h"
#include "qdict.h"
#include "qqueue.h"
#include "qguardedptr.h"
#include "qclipboard.h"
#include "qbitmap.h"
#include "qwssocket_qws.h"
#include "qwsevent_qws.h"
#include "qwscommand_qws.h"
#include "qwsproperty_qws.h"
#include "qgfx_qws.h"
#include "qfontmanager_qws.h"
#include "qcopchannel_qws.h"
#include "qlock_qws.h"
#include "qmemorymanager_qws.h"
#include "qwsmanager_qws.h"
#include "qwsregionmanager_qws.h"
#include "qwindowsystem_qws.h"
#include "qwsdisplay_qws.h"
#include "qnetwork.h"
#include "qcursor.h"
#include "qsound.h"
#include "qinterlacestyle.h"
#include "qinputcontext_p.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#ifndef QT_NO_QWS_MULTIPROCESS
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/socket.h>
#endif

#include <stdlib.h>
#ifndef QT_NO_SM_SUPPORT
#include <pwd.h>
#endif
#include <ctype.h>
#include <locale.h>
#include <errno.h>

#include "qfile.h"

#if defined(_OS_IRIX_)
#include <bstring.h>
#endif

#include <sys/time.h>

#if defined(_OS_AIX_) && defined(_CC_GNU_)
#include <sys/select.h>
#include <unistd.h>
#endif

#if defined(_OS_QNX_)
#include <sys/select.h>
#endif

#if defined(QT_THREAD_SUPPORT)
#include "qvaluestack.h"
#include "qthread.h"
#endif

#if defined(_OS_UNIX_) && defined(QT_THREAD_SUPPORT)
#include <sys/ioctl.h>
static int qt_thread_pipe[2];
#endif

const int qwsSharedRamSize = 32 * 1024;	//Small amount to fit on small devices.

// These are in qapplication.cpp in qt/main
static bool servermaxrect=FALSE;
Q_EXPORT QRect qt_maxWindowRect;
void qt_setMaxWindowRect(const QRect& r)
{
    qt_maxWindowRect = qt_screen->mapFromDevice(r,
	QSize(qt_screen->deviceWidth(), qt_screen->deviceHeight()) );
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
QLock *QWSDisplay::lock = 0;

extern bool qt_is_gui_used;
extern int qt_widget_tlw_gravity;
extern QApplication::Type qt_appType;
extern void qwsUpdateActivePainters();

//these used to be environment variables, they are initialized from
//environment variables in

bool qws_savefonts = FALSE;
bool qws_screen_is_interlaced=FALSE; //### should be detected
bool qws_shared_memory = FALSE;
bool qws_sw_cursor = TRUE;
bool qws_accel = TRUE;	    // ### never set
const char *qws_display_spec = ":0";
int qws_display_id = 0;
int qws_client_id = 0;
QWidget *qt_pressGrab = 0;
QWidget *qt_mouseGrb = 0;
int *qt_last_x = 0;
int *qt_last_y = 0;
int *qt_next_pixmap_id = 0;
bool qws_overrideCursor = FALSE;
static bool qws_regionRequest = FALSE;
#ifndef QT_NO_QWS_MANAGER
static QWSDecoration *qws_decoration = 0;
#endif
#undef gettimeofday
extern "C" int gettimeofday( struct timeval *, struct timezone * );
#undef select
extern "C" int select( int, void *, void *, void *, struct timeval * );

#ifdef DEBUG
/*
extern "C" void dumpmem(const char* m)
{
    static int init=0;
    static int prev=0;
    FILE* f = fopen("/proc/meminfo","r");
    //    char line[100];
    int total=0,used=0,free=0,shared=0,buffers=0,cached=0;
    fscanf(f,"%*[^M]Mem: %d %d %d %d %d %d",&total,&used,&free,&shared,&buffers,&cached);
    used -= buffers + cached;
    if (!init) {
        init=used;
    } else {
        printf("%40s: %+8d = %8d\n",m,used-init-prev,used-init);
        prev = used-init;
    }
    fclose(f);
}
*/
#endif

// Get the name of the directory where Qt/Embedded temporary data should
// live.
QString qws_dataDir()
{
   
    QString idName("0"); 
    const char *display = getenv("QWS_DISPLAY");
    if ( display ){
	QRegExp r( ":[0-9]+" );  // supports more than 10 displays
	int len;
	int m = r.match( QString(display) , 0, &len );
	if ( m >= 0 ) 
	    idName = QString(display).mid( m+1, len-1 );
    }

    QString dataDir = "/tmp/qtembedded-" + idName;
    if ( mkdir( dataDir.latin1(), 0700 ) ) {
	if ( errno != EEXIST ) {
	    qFatal( QString("Cannot create Qt/Embedded data directory: %1")
		    .arg( dataDir ) );
	}
    }

    struct stat buf;
    if ( lstat( dataDir.latin1(), &buf ) )
	qFatal( QString( "stat failed for Qt/Embedded data directory: %1" )
		.arg( dataDir ) );

    if ( !S_ISDIR( buf.st_mode ) )
	qFatal( QString( "%1 is not a directory" ).arg( dataDir ) );

    if ( buf.st_uid != getuid() )
	qFatal( QString( "Qt/Embedded data directory is not owned by user %1: %2" )
		.arg( getuid() ).arg( dataDir ) );

    if ( (buf.st_mode & 0677) != 0600 )
	qFatal( QString( "Qt/Embedded data directory has incorrect permissions: %1" )
		.arg( dataDir ) );

    dataDir += "/";

    return dataDir;
}

// Get the filename of the pipe Qt/Embedded uses for server/client comms
QString qws_qtePipeFilename()
{
    return (qws_dataDir() + QString(QTE_PIPE).arg(qws_display_id));
}


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

class QWSDisplay;

static const char *appName;			// application name
static const char *appFont	= 0;		// application font
static const char *appBGCol	= 0;		// application bg color
static const char *appFGCol	= 0;		// application fg color
static const char *appBTNCol	= 0;		// application btn color
static const char *mwGeometry	= 0;		// main widget geometry
static const char *mwTitle	= 0;		// main widget title
//static bool	mwIconic	= FALSE;	// main widget iconified

static bool	app_do_modal	= FALSE;	// modal mode
QWSDisplay*	qt_fbdpy = 0;			// QWS `display'
static fd_set	app_readfds;			// fd set for reading
static fd_set	app_writefds;			// fd set for writing
static fd_set	app_exceptfds;			// fd set for exceptions

static int	mouseButtonPressed   = 0;	// last mouse button pressed
static int	mouseButtonState     = 0;	// mouse button state
static int	mouseButtonPressTime = 0;	// when was a button pressed
static short	mouseXPos, mouseYPos;		// mouse position in act window

extern QWidgetList *qt_modal_stack;		// stack of modal widgets

static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;
static bool	    popupGrabOk;
static QGuardedPtr<QWidget> *mouseInWidget = 0;

static bool sm_blockUserInput = FALSE;		// session management

// one day in the future we will be able to have static objects in libraries....
static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups
#ifdef QT_KEYPAD_MODE
static bool editingBeforePopup = FALSE;
extern bool qt_modalEditingEnabled;
#endif


typedef void (*VFPTR)();
typedef QValueList<VFPTR> QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

void qt_install_preselect_handler( VFPTR );
void qt_remove_preselect_handler( VFPTR );
static QVFuncList *qt_preselect_handler = 0;
void qt_install_postselect_handler( VFPTR );
void qt_remove_postselect_handler( VFPTR );
static QVFuncList *qt_postselect_handler = 0;
void qt_install_preselect_handler( VFPTR handler )
{
    if ( !qt_preselect_handler )
	qt_preselect_handler = new QVFuncList;
    qt_preselect_handler->append( handler );
}
void qt_remove_preselect_handler( VFPTR handler )
{
    if ( qt_preselect_handler ) {
	QVFuncList::Iterator it = qt_preselect_handler->find( handler );
	if ( it != qt_preselect_handler->end() )
		qt_preselect_handler->remove( it );
    }
}
void qt_install_postselect_handler( VFPTR handler )
{
    if ( !qt_postselect_handler )
	qt_postselect_handler = new QVFuncList;
    qt_postselect_handler->prepend( handler );
}
void qt_remove_postselect_handler( VFPTR handler )
{
    if ( qt_postselect_handler ) {
	QVFuncList::Iterator it = qt_postselect_handler->find( handler );
	if ( it != qt_postselect_handler->end() )
		qt_postselect_handler->remove( it );
    }
}

static void	initTimers();
static void	cleanupTimers();
static timeval	watchtime;			// watch if time is turned back
timeval		*qt_wait_timer();
timeval 	*qt_wait_timer_max = 0;
int	        qt_activate_timers();

QObject	       *qt_clipboard = 0;

QWidget	       *qt_button_down	 = 0;		// widget got last button-down
WId qt_last_cursor = 0xffffffff;  // Was -1, but WIds are unsigned

class QWSMouseEvent;
class QWSKeyEvent;

class QETWidget : public QWidget		// event translator widget
{
public:
    void setWState( WFlags f )		{ QWidget::setWState(f); }
    void clearWState( WFlags f )	{ QWidget::clearWState(f); }
    void setWFlags( WFlags f )		{ QWidget::setWFlags(f); }
    void clearWFlags( WFlags f )	{ QWidget::clearWFlags(f); }
    bool isFullScreen() { return isTopLevel() && topData()->fullscreen; }
    bool translateMouseEvent( const QWSMouseEvent * );
    bool dispatchMouseEvent( const QWSMouseEvent * );

    bool translateKeyEvent( const QWSKeyEvent *, bool grab );
    bool translateRegionModifiedEvent( const QWSRegionModifiedEvent * );
    bool translateWheelEvent( int global_x, int global_y, int delta, int state );
    void repaintHierarchy(QRegion r, bool post);
    void repaintDecoration(QRegion r, bool post);
    void updateRegion();

    bool raiseOnClick()
    {
	// With limited windowmanagement/taskbar/etc., raising big windows
	// (eg. spreadsheet) over the top of everything else (eg. calculator)
	// is just annoying.
	return !isMaximized() && !topData()->fullscreen;
    }
};

// Single-process stuff. This should maybe move into qwindowsystem_qws.cpp

static bool qws_single_process;
static QQueue<QWSEvent> incoming;
static QQueue<QWSCommand> outgoing;

void qt_client_enqueue(const QWSEvent *event )
{
    QWSEvent *copy = QWSEvent::factory( event->type );
    copy->copyFrom( event );
    incoming.enqueue( copy );
}

QQueue<QWSCommand> *qt_get_server_queue()
{
    return &outgoing;
}

void qt_server_enqueue( const QWSCommand *command )
{
    QWSCommand *copy = QWSCommand::factory( command->type );
    copy->copyFrom( command );
    outgoing.enqueue( copy );
}

class QWSDisplayData {
public:
    QWSDisplayData( QObject* parent, bool singleProcess = FALSE )
    {
#ifndef QT_NO_QWS_MULTIPROCESS
	if ( singleProcess )
	    csocket = 0;
	else {
	    csocket = new QWSSocket(parent);
	    QObject::connect( csocket, SIGNAL(connectionClosed()),
			      qApp, SLOT(quit()) );
	}
#endif
	init();
    }

    ~QWSDisplayData()
    {
	delete rgnMan; rgnMan = 0;
	delete memorymanager; memorymanager = 0;
	qt_screen->disconnect();
	delete qt_screen; qt_screen = 0;
#ifndef QT_NO_QWS_CURSOR
	delete qt_screencursor; qt_screencursor = 0;
#endif
#ifndef QT_NO_QWS_MULTIPROCESS
	shmdt((char*)sharedRam);
	if ( !csocket && ramid != -1 ) {
	    shmctl( ramid, IPC_RMID, 0 );
	}
	if ( csocket ) {
	    csocket->flush(); // may be pending QCop message, eg.
	    delete csocket;
	}
#endif
    }

    void flush()
    {
#ifndef QT_NO_QWS_MULTIPROCESS
	if ( csocket )
	    csocket->flush();
#endif
    }

    //####public data members

    QWSRegionManager *rgnMan;
    uchar *sharedRam;
    int sharedRamSize;
    int ramid;
    int focusWin;

private:
#ifndef QT_NO_QWS_MULTIPROCESS
    QWSSocket *csocket;
#endif
    QList<QWSEvent> queue;

    QWSConnectedEvent* connected_event;
    QWSMouseEvent* mouse_event;
    QWSRegionModifiedEvent *region_event;
    QWSRegionModifiedEvent *region_ack;
    QPoint region_offset;
    int region_offset_window;
#ifndef QT_NO_COP
    QWSQCopMessageEvent *qcop_response;
#endif
    QWSEvent* current_event;
    QValueList<int> unused_identifiers;
    int mouse_event_count;

    enum { VariableEvent=-1 };
public:
    bool queueNotEmpty()
    {
	return mouse_event||region_event||queue.count() > 0;
    }
    QWSEvent *dequeue()
    {
	QWSEvent *r;
	if ( queue.count() ) {
	    r = queue.first(); queue.removeFirst();
	} else if ( mouse_event ) {
	    r = mouse_event;
	    mouse_event = 0;
	    mouse_event_count = 0;
	} else {
	    r = region_event;
	    region_event = 0;
	}
	return r;
    }

    QWSEvent *peek() {
	return queue.first();
    }
#ifndef QT_NO_QWS_MULTIPROCESS
    bool directServerConnection() { return csocket == 0; }
#else
    bool directServerConnection() { return TRUE; }
#endif
    void fillQueue();
    void waitForConnection();
    void waitForRegionAck();
    void waitForGetFocus();
    void waitForCreation();
#ifndef QT_NO_COP
    void waitForQCopResponse();
#endif
    void offsetPendingExpose( int, const QPoint & );
    void translateExpose( QWSRegionModifiedEvent *re, const QPoint &p )
    {
	for ( int i = 0; i < re->simpleData.nrectangles; i++ )
	    re->rectangles[i].moveBy(p.x(), p.y());
    }
    void init();
    void create()
    {
	QWSCreateCommand cmd;
#ifndef QT_NO_QWS_MULTIPROCESS
	if  ( csocket )
	    cmd.write( csocket );
	else
#endif
	    qt_server_enqueue( &cmd );
    }

    void sendCommand( QWSCommand & cmd )
    {
#ifndef QT_NO_QWS_MULTIPROCESS
	if  ( csocket )
	    cmd.write( csocket );
	else
#endif
	    qt_server_enqueue( &cmd );
    }


    QWSEvent *readMore();



    int takeId()
    {
	// top up bag
	create();
	if ( !unused_identifiers.count() ) {
	    // We have to wait!
	    for (int o=0; o<30; o++)
		create();
	    waitForCreation();
	}
	QValueList<int>::Iterator head = unused_identifiers.begin();
	int i = *head;
	unused_identifiers.remove(head);
	return i;
    }
};



void QWSDisplayData::init()
{
    connected_event = 0;
    region_ack = 0;
    mouse_event = 0;
    region_event = 0;
    region_offset_window = 0;
#ifndef QT_NO_COP
    qcop_response = 0;
#endif
    current_event = 0;
    mouse_event_count = 0;
    focusWin = 0;
    ramid = -1;

    QString pipe = qws_qtePipeFilename();

#ifndef QT_NO_QWS_MULTIPROCESS
    key_t memkey =  ftok( pipe.latin1(), 'm' );


    if ( csocket )    {
	// QWS client
	csocket->connectToLocalFile(pipe);
	QWSIdentifyCommand cmd;
	cmd.setId(appName);
#ifndef QT_NO_QWS_MULTIPROCESS
	if  ( csocket )
	    cmd.write( csocket );
	else
#endif
	    qt_server_enqueue( &cmd );

	// wait for connect confirmation
	waitForConnection();

	qws_client_id = connected_event->simpleData.clientId;

	// now we want to get the exact display spec to use if we haven't
	// specified anything.
	if ( qws_display_spec[0] == ':' )
	    qws_display_spec = connected_event->display;

	if ( !QWSDisplay::initLock( pipe, FALSE ) )
	    qFatal( "Cannot get display lock" );
	int ramid = shmget(memkey,0,0);
	if ( ramid == -1 ) {
	    perror( "Cannot find main shared memory" );
	    exit(1);
	}
	struct shmid_ds shminfo;
	if ( shmctl( ramid, IPC_STAT, &shminfo ) == -1 )
	    qFatal( "Cannot get main ram memory status" );
	sharedRamSize=shminfo.shm_segsz;

	sharedRam = (uchar *)shmat(ramid,0,0);
	if(sharedRam==(uchar *)-1) {
	    perror("Can't attach to main ram memory.");
	    exit(1);
	}
	qt_get_screen( qws_display_id, qws_display_spec );
    }
    else
#endif
    {

	// QWS server
	if ( !QWSDisplay::initLock( pipe, TRUE ) )
	    qFatal( "Cannot get display lock" );

	sharedRamSize = qwsSharedRamSize;

#ifndef QT_NO_QWS_MULTIPROCESS

	key_t memkey = ftok( pipe.latin1(), 'm' );
	ramid=shmget(memkey,sharedRamSize,IPC_CREAT|0600);
	if(ramid<0) {
	    perror("Cannot allocate main ram shared memory\n");
	}
	sharedRam=(uchar *)shmat(ramid,0,0);
	if(sharedRam==(uchar *)-1) {
	    perror("Cannot attach to main ram shared memory\n");
	}
#else
	sharedRam=(uchar *)malloc(sharedRamSize);
#endif
	// Need to zero index count at end of block, might as well zero
	// the rest too
	memset(sharedRam,0,sharedRamSize);

	QScreen *s = qt_get_screen( qws_display_id, qws_display_spec );
	s->initDevice();

	QWSIdentifyCommand cmd;
	cmd.setId(appName);
	qt_server_enqueue( &cmd );
    }
    qt_setMaxWindowRect(QRect(0,0,qt_screen->width(),qt_screen->height()));
    int mouseoffset = 0;

#ifndef QT_NO_QWS_CURSOR
    mouseoffset=qt_screen->initCursor(sharedRam + sharedRamSize,
#ifndef QT_NO_QWS_MULTIPROCESS
				      !csocket
#else
				      TRUE
#endif
				      );
#endif

    sharedRamSize -= mouseoffset;
    sharedRamSize -= sizeof(int);
    qt_last_x = (int *)(sharedRam + sharedRamSize);
    sharedRamSize -= sizeof(int);
    qt_last_y = (int *)(sharedRam + sharedRamSize);
    sharedRamSize -= sizeof(int);
    qt_next_pixmap_id = (int *)(sharedRam + sharedRamSize);
#ifndef QT_NO_QWS_MULTIPROCESS
    if (!csocket)
	*qt_next_pixmap_id = 1;
#endif

    memorymanager = new QMemoryManager();

#ifdef QT_NO_QWS_MULTIPROCESS
    rgnMan = new QWSRegionManager( pipe, 0 ); //####### not necessary
#else
    rgnMan = new QWSRegionManager( pipe, csocket );
    if ( csocket )
	csocket->flush();
#endif
}


QWSEvent* QWSDisplayData::readMore()
{
#ifdef QT_NO_QWS_MULTIPROCESS
    return incoming.dequeue();
#else
    if ( !csocket )
	return incoming.dequeue();
    // read next event
    if ( !current_event ) {
	int event_type = qws_read_uint( csocket );

	if ( event_type >= 0 ) {
	    current_event = QWSEvent::factory( event_type );
	}
    }

    if ( current_event ) {
	if ( current_event->read( csocket ) ) {
	    // Finished reading a whole event.
	    QWSEvent* result = current_event;
	    current_event = 0;
	    return result;
	}
    }

    // Not finished reading a whole event.
    return 0;
#endif
}


void QWSDisplayData::fillQueue()
{
    QWSServer::processEventQueue();
    QWSEvent *e = readMore();
    while ( e ) {
	if ( e->type == QWSEvent::Connected ) {
	    connected_event = (QWSConnectedEvent *)e;
	    return;
  	} else if ( e->type == QWSEvent::Creation ) {
	    QWSCreationEvent *ce = (QWSCreationEvent*)e;
	    unused_identifiers.append(ce->simpleData.objectid);
	    delete e;
	} else if ( e->type == QWSEvent::Mouse ) {
	    if ( !qt_screen ) {
		delete e;
	    } else {
		QWSMouseEvent *me = (QWSMouseEvent*)e;
		if ( mouse_event ) {
		    if ( (mouse_event->window() != e->window ()
			  || mouse_event->simpleData.state !=
			  me->simpleData.state )) {
			queue.append( mouse_event );
			mouse_event_count = 0;
		    } else if ( mouse_event_count == 1 ) {
			// make sure the position of the press is not
			// compressed away.
			queue.append( mouse_event );
		    } else {
			delete mouse_event;
		    }
		}
		QSize s( qt_screen->deviceWidth(), qt_screen->deviceHeight() );
		QPoint p(me->simpleData.x_root, me->simpleData.y_root);
		p = qt_screen->mapFromDevice( p, s );
		me->simpleData.x_root = p.x();
		me->simpleData.y_root = p.y();
		mouse_event = me;
		mouse_event_count++;
	    }
	} else if ( e->type == QWSEvent::RegionModified ) {
	    QWSRegionModifiedEvent *re = (QWSRegionModifiedEvent *)e;
	    if ( re->simpleData.is_ack ) {
		region_ack = re;
		region_offset = QPoint();
		region_offset_window = 0;
	    } else {
		if ( region_offset_window == re->window() && !region_offset.isNull() ) {
//		    qDebug( "Rgn Adjust a %d, %d", region_offset.x(), region_offset.y() );
		    translateExpose( re, region_offset );
		}
		if ( (!region_event || re->window() == region_event->window() ) ) {
		    if ( region_event ) {
			QRegion r1;
			r1.setRects( re->rectangles, re->simpleData.nrectangles );
			QRegion r2;
			r2.setRects( region_event->rectangles,
				region_event->simpleData.nrectangles );
			QRegion ur( r1 + r2 );
			region_event->setData( (char *)ur.rects().data(),
				ur.rects().count() * sizeof(QRect), TRUE );
			region_event->simpleData.nrectangles = ur.rects().count();
			delete e;
		    } else {
			region_event = re;
		    }
		} else {
		    queue.append(e);
		}
	    }
	} else if ( e->type==QWSEvent::MaxWindowRect && !servermaxrect ) {
	    // Process this ASAP, in case new widgets are created (startup)
	    servermaxrect=TRUE;
	    QRect r = ((QWSMaxWindowRectEvent*)e)->simpleData.rect;
	    extern void qt_setMaxWindowRect(const QRect& r);
	    qt_setMaxWindowRect(r);
	    delete e;
	} else if ( e->type == QWSEvent::Focus ) {
	    QWSFocusEvent *fe = (QWSFocusEvent*)e;
	    if (focusWin && fe->simpleData.window == focusWin) {
		QListIterator<QWSEvent> it(queue);
		for (; it.current(); ++it) {
		    QWSEvent *qe = (QWSEvent *)it.current();
		    ++it;
		    if (qe->type == QWSEvent::Focus) {
			QWSFocusEvent *ofe = (QWSFocusEvent *)qe;
			if (ofe->simpleData.window == focusWin
			    || ofe->simpleData.get_focus == TRUE)
			    queue.removeRef(qe);
		    }
		}
		queue.prepend(e);
		focusWin = 0;
	    } else {
		queue.append(e);
	    }
#ifndef QT_NO_COP
	} else if ( e->type == QWSEvent::QCopMessage ) {
	    QWSQCopMessageEvent *pe = (QWSQCopMessageEvent*)e;
	    if ( pe->simpleData.is_response ) {
		qcop_response = pe;
	    } else {
		queue.append(e);
	    }
#endif
	} else {
	    queue.append(e);
	}
	e = readMore();
    }
}

void QWSDisplayData::offsetPendingExpose( int window, const QPoint &offset )
{
    if ( offset.isNull() )
	return;

    region_offset = offset;
    region_offset_window = window;

    QListIterator<QWSEvent> it(queue);
    for ( ; it.current(); ++it ) {
	QWSEvent *e = it.current();
	if ( e->type == QWSEvent::RegionModified ) {
	    QWSRegionModifiedEvent *re = (QWSRegionModifiedEvent *)e;
	    if ( !re->simpleData.is_ack && region_offset_window == re->window() ) {
//		qDebug( "Rgn Adjust b %d, %d", region_offset.x(), region_offset.y() );
		translateExpose( re, region_offset );
	    }
	}
    }

    if ( region_event && region_offset_window == region_event->window() ) {
//	qDebug( "Rgn Adjust c %d, %d", region_offset.x(), region_offset.y() );
	translateExpose( region_event, region_offset );
    }
}

void QWSDisplayData::waitForConnection()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    for ( int i = 0; i < 5; i++ ) {
	if ( csocket ) {
	    csocket->flush();
	    csocket->waitForMore(2000);
	}
	fillQueue();
	if ( connected_event )
	    return;
	usleep( 50000 );
    }
#else
    if ( connected_event )
	return;
#endif
    qWarning("No Qt/Embedded server appears to be running.");
    qWarning("If you want to run %s as a server,",appName);
    qWarning("add the \"-qws\" command-line option.");
    exit(1);
}


void QWSDisplayData::waitForRegionAck()
{
    for (;;) {
	fillQueue();
	if ( region_ack )
	    break;
#ifndef QT_NO_QWS_MULTIPROCESS
	if ( csocket ) {
	    csocket->flush();
	    csocket->waitForMore(1000);
	    if ( csocket->state() != QSocket::Connection )
		return;
	}
#endif
    }
    queue.prepend(region_ack);
    region_ack = 0;
}

void QWSDisplayData::waitForGetFocus()
{
    for (;;) {
	fillQueue();
	if ( !focusWin )
	    break;
#ifndef QT_NO_QWS_MULTIPROCESS
	if ( csocket ) {
	    csocket->flush();
	    csocket->waitForMore(1000);
	    if ( csocket->state() != QSocket::Connection )
		return;
	}
#endif
    }
}

void QWSDisplayData::waitForCreation()
{
    fillQueue();
    while ( unused_identifiers.count() == 0 ) {
#ifndef QT_NO_QWS_MULTIPROCESS
	if ( csocket ) {
	    csocket->flush();
	    csocket->waitForMore(1000);
	}
#endif
	fillQueue();
    }
}

#ifndef QT_NO_COP
void QWSDisplayData::waitForQCopResponse()
{
    for (;;) {
	fillQueue();
	if ( qcop_response )
	    break;
#ifndef QT_NO_QWS_MULTIPROCESS
	if ( csocket ) {
	    csocket->flush();
	    csocket->waitForMore(1000);
	}
#endif
    }
    queue.prepend(qcop_response);
    qcop_response = 0;
}
#endif


QWSDisplay::QWSDisplay()
{
    d = new QWSDisplayData( 0, qws_single_process );
}

QWSDisplay::~QWSDisplay()
{
    delete d;
    delete lock;
}

QGfx * QWSDisplay::screenGfx()
{
    return qt_screen->screenGfx();
}

QWSRegionManager *QWSDisplay::regionManager() const
{
    return d->rgnMan;
}

bool QWSDisplay::eventPending() const
{
#ifndef QT_NO_QWS_MULTIPROCESS
    d->flush();
#endif
    d->fillQueue();
    return d->queueNotEmpty();
}


/*
  Caller must delete return value!
 */
QWSEvent*  QWSDisplay::getEvent()
{
    d->fillQueue();
    ASSERT(d->queueNotEmpty());
    QWSEvent* e = d->dequeue();

    return e;
}

uchar* QWSDisplay::frameBuffer() const { return qt_screen->base(); }
int QWSDisplay::width() const { return qt_screen->width(); }
int QWSDisplay::height() const { return qt_screen->height(); }
int QWSDisplay::depth() const { return qt_screen->depth(); }
int QWSDisplay::pixmapDepth() const { return qt_screen->pixmapDepth(); }
bool QWSDisplay::supportsDepth(int d) const { return qt_screen->supportsDepth(d); }
uchar *QWSDisplay::sharedRam() const { return d->sharedRam; }
int QWSDisplay::sharedRamSize() const { return d->sharedRamSize; }


void QWSDisplay::addProperty( int winId, int property )
{
    QWSAddPropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    d->sendCommand( cmd );
}

void QWSDisplay::setProperty( int winId, int property, int mode, const QByteArray &data )
{
    QWSSetPropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    cmd.simpleData.mode = mode;
    cmd.setData( data.data(), data.size() );
    d->sendCommand( cmd );
}

void QWSDisplay::setProperty( int winId, int property, int mode,
			      const char * data )
{
    QWSSetPropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    cmd.simpleData.mode = mode;
    cmd.setData( (char *)data, strlen(data) );
    d->sendCommand( cmd );
}

void QWSDisplay::removeProperty( int winId, int property )
{
    QWSRemovePropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    d->sendCommand( cmd );
}

/*!
  It is the caller's responsibility to delete[] \a data.

 */
bool QWSDisplay::getProperty( int winId, int property, char *&data, int &len )
{
    QWSGetPropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    d->sendCommand( cmd );

    getPropertyLen = -2;
    getPropertyData = 0;

    while ( getPropertyLen == -2 )
	qApp->processEvents(); //########## USE an ACK event instead. That's dangerous!

    len = getPropertyLen;
    data = getPropertyData;

    getPropertyLen = -2;
    getPropertyData = 0;

    return len != -1;
}

void QWSDisplay::setAltitude(int winId, int alt, bool fixed )
{
    QWSChangeAltitudeCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.altitude = alt;
    cmd.simpleData.fixed = fixed;
    if ( d->directServerConnection() ) {
	qwsServer->set_altitude( &cmd );
    } else {
	d->sendCommand( cmd );
    }
    d->waitForRegionAck();
}

void QWSDisplay::requestFocus(int winId, bool get)
{
    QWSRequestFocusCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.flag = get;
    if ( d->directServerConnection() )
	qwsServer->request_focus( &cmd );
    else
	d->sendCommand( cmd );
    if (get) {
	d->focusWin = winId;
	d->waitForGetFocus();
    }
}

void QWSDisplay::setIdentity(const QString &appName)
{
    QWSIdentifyCommand cmd;
    cmd.setId(appName);
    if ( d->directServerConnection() )
	qwsServer->set_identity( &cmd );
    else
	d->sendCommand( cmd );
}

void QWSDisplay::nameRegion(int winId, const QString& n, const QString &c)
{
    QWSRegionNameCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.setName(n, c);
    if ( d->directServerConnection() )
	qwsServer->name_region( &cmd );
    else
	d->sendCommand( cmd );
}

void QWSDisplay::requestRegion(int winId, QRegion r)
{
    qws_regionRequest = TRUE;
    if ( d->directServerConnection() ) {
	qwsServer->request_region( winId, r );
    } else {
	//by sending the event, I promise not to paint outside the region
	//QETWidget *widget = (QETWidget*)QWidget::find( (WId)winId );

	QArray<QRect> ra = r.rects();

	/*
	  for ( int i = 0; i < ra.size(); i++ ) {
	  QRect r( ra[i] );
	  qDebug("rect: %d %d %d %d", r.x(), r.y(), r.right(), r.bottom() );
	  }
	*/

	QWSRegionCommand cmd;
	cmd.simpleData.windowid = winId;
	cmd.simpleData.nrectangles = ra.count();
	cmd.setData( (char *)ra.data(), ra.count() * sizeof(QRect), FALSE);
	d->sendCommand( cmd );
    }
    if ( !r.isEmpty() )
	d->waitForRegionAck();
}

void QWSDisplay::moveRegion( int winId, int dx, int dy )
{
    //UNUSED QETWidget *widget = (QETWidget*)QWidget::find( (WId)winId );

    QPoint p1 = qt_screen->mapToDevice( QPoint(0, 0),
		    QSize(qt_screen->width(), qt_screen->height()) );
    QPoint p2 = qt_screen->mapToDevice( QPoint(dx, dy),
		    QSize(qt_screen->width(), qt_screen->height()) );

    QWSRegionMoveCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.dx = p2.x() - p1.x();
    cmd.simpleData.dy = p2.y() - p1.y();

    if ( d->directServerConnection() ) {
	qwsServer->move_region( &cmd );
    } else {
	d->sendCommand( cmd );
    }
    d->offsetPendingExpose( winId, QPoint(cmd.simpleData.dx, cmd.simpleData.dy) );
    d->waitForRegionAck();
}

void QWSDisplay::destroyRegion( int winId )
{
    QWSRegionDestroyCommand cmd;
    cmd.simpleData.windowid = winId;
    if ( d->directServerConnection() ) {
	qwsServer->destroy_region( &cmd );
    } else {
	d->sendCommand( cmd );
    }
}


#ifndef QT_NO_QWS_IM
void QWSDisplay::setIMInfo( int winId, int x, int y, const QRect &r, bool reset)
{
    QWSSetIMInfoCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.x = x;
    cmd.simpleData.y = y;
    cmd.simpleData.x1 = r.x();
    cmd.simpleData.y1 = r.y();
    cmd.simpleData.w = r.width();
    cmd.simpleData.h = r.height();
    cmd.simpleData.reset = reset;

    if ( d->directServerConnection() ) {
	qwsServer->set_im_info( &cmd );
    } else {
	d->sendCommand( cmd );
    }
}

void QWSDisplay::resetIM()
{
    QWSResetIMCommand cmd;
    if ( d->directServerConnection() ) {
	qwsServer->reset_im( &cmd );
    } else {
	d->sendCommand( cmd );
    }
}

void QWSDisplay::sendIMMouseEvent( int index, bool isPress )
{
    QWSIMMouseCommand cmd;
    cmd.simpleData.index = index;
    cmd.simpleData.state = isPress ? QWSServer::MousePress : QWSServer::MouseRelease;
    if ( d->directServerConnection() ) {
	qwsServer->send_im_mouse( &cmd );
    } else {
	d->sendCommand( cmd );
    }
}

void QWSDisplay::setInputFont( int winId, const QFont &f )
{
    static QFont prevFont;
    if ( prevFont != f ) {
	prevFont = f;
	QWSSetIMFontCommand cmd;
	cmd.simpleData.windowid = winId;
	cmd.setFont( f );
	if ( d->directServerConnection() ) {
	    qwsServer->set_im_font( &cmd );
	} else {
	    d->sendCommand( cmd );
	}
    }


}

#endif

int QWSDisplay::takeId()
{
    return d->takeId();
}

bool QWSDisplay::initLock( const QString &filename, bool create )
{
    if ( !lock ) {
	lock = new QLock( filename, 'd', create );

	if ( !lock->isValid() ) {
	    delete lock;
	    lock = 0;
	    return FALSE;
	}
    }

    return TRUE;
}

void QWSDisplay::setSelectionOwner( int winId, const QTime &time )
{
    QWSSetSelectionOwnerCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.hour = time.hour();
    cmd.simpleData.minute = time.minute();
    cmd.simpleData.sec = time.second();
    cmd.simpleData.ms = time.msec();
    d->sendCommand( cmd );
}

void QWSDisplay::convertSelection( int winId, int selectionProperty, const QString &mimeTypes )
{
#ifndef QT_NO_QWS_PROPERTIES
    // ### we need the atom/property thingy like in X here
    addProperty( winId, QT_QWS_PROPERTY_CONVERTSELECTION );
    setProperty( winId, QT_QWS_PROPERTY_CONVERTSELECTION,
		 (int)QWSPropertyManager::PropReplace, QCString( mimeTypes.latin1() ) );
#endif
    QWSConvertSelectionCommand cmd;
    cmd.simpleData.requestor = winId;
    cmd.simpleData.selection = selectionProperty;
    cmd.simpleData.mimeTypes = QT_QWS_PROPERTY_CONVERTSELECTION;
    d->sendCommand( cmd );
}

void QWSDisplay::defineCursor(int id, const QBitmap &curs, const QBitmap &mask,
			    int hotX, int hotY)
{
    QImage cursImg = curs.convertToImage();
    QImage maskImg = mask.convertToImage();

    QWSDefineCursorCommand cmd;
    cmd.simpleData.width = curs.width();
    cmd.simpleData.height = curs.height();
    cmd.simpleData.hotX = hotX;
    cmd.simpleData.hotY = hotY;
    cmd.simpleData.id = id;

    int dataLen = cursImg.numBytes();

    unsigned char *data = new unsigned char [dataLen*2];
    memcpy(data, cursImg.bits(), dataLen);
    memcpy(data + dataLen, maskImg.bits(), dataLen);

    cmd.setData( (char*)data, dataLen*2 );
    delete [] data;
    d->sendCommand( cmd );
}

#ifndef QT_NO_SOUND
void QWSDisplay::playSoundFile(const QString& f)
{
    QWSPlaySoundCommand cmd;
    cmd.setFileName(f);
    d->sendCommand( cmd );
}
#endif

#ifndef QT_NO_COP
void QWSDisplay::registerChannel( const QCString& channel )
{
    QWSQCopRegisterChannelCommand reg;
    reg.setChannel( channel );
    qt_fbdpy->d->sendCommand( reg );
}

void QWSDisplay::sendMessage(const QCString &channel, const QCString &msg,
		   const QByteArray &data )
{
    QWSQCopSendCommand com;
    com.setMessage( channel, msg, data );
    qt_fbdpy->d->sendCommand( com );
}

/*
  caller deletes result
*/
QWSQCopMessageEvent* QWSDisplay::waitForQCopResponse()
{
    qt_fbdpy->d->waitForQCopResponse();
    QWSQCopMessageEvent *e = (QWSQCopMessageEvent*)qt_fbdpy->d->dequeue();
    ASSERT( e->type == QWSEvent::QCopMessage );
    return e;
}
#endif


void QWSDisplay::setCaption( QWidget *w, const QString &c )
{
    if ( w->isTopLevel() ) {
	nameRegion( w->winId(), w->name(), c );
	((QETWidget *)w)->repaintDecoration(qApp->desktop()->rect(), TRUE);
    }
}

void QWSDisplay::selectCursor( QWidget *w, unsigned int cursId )
{
    if (cursId != qt_last_cursor)
    {
	QWidget *top = w->topLevelWidget();
	qt_last_cursor = cursId;
	QWSSelectCursorCommand cmd;
	cmd.simpleData.windowid = top->winId();
	cmd.simpleData.id = cursId;
	d->sendCommand( cmd );
	d->flush();
    }
}

void QWSDisplay::moveCursor( int x, int y )
{
    QWSMoveCursorCommand cmd;
    cmd.simpleData.x = x;
    cmd.simpleData.y = y;
    if ( d->directServerConnection() ) {
	qwsServer->invokeMoveCursor( &cmd );
    } else {
	d->sendCommand( cmd );
	d->flush();
    }
}

void QWSDisplay::grabMouse( QWidget *w, bool grab )
{
    QWidget *top = w->topLevelWidget();
    QWSGrabMouseCommand cmd;
    cmd.simpleData.windowid = top->winId();
    cmd.simpleData.grab = grab;
    d->sendCommand( cmd );
    d->flush();
}

void QWSDisplay::grabKeyboard( QWidget *w, bool grab )
{
    QWidget *top = w->topLevelWidget();
    QWSGrabKeyboardCommand cmd;
    cmd.simpleData.windowid = top->winId();
    cmd.simpleData.grab = grab;
    d->sendCommand( cmd );
    d->flush();
}

QList<QWSWindowInfo> * QWSDisplay::windowList()
{
    QList<QWSWindowInfo> * ret=new QList<QWSWindowInfo>;
    ret->setAutoDelete(true);
    if(d->directServerConnection()) {
	QList<QWSInternalWindowInfo> * qin=QWSServer::windowList();
	QWSInternalWindowInfo * qwi;
	for(qwi=qin->first();qwi!=0;qwi=qin->next()) {
	    QWSWindowInfo * tmp=new QWSWindowInfo();
	    tmp->winid=qwi->winid;
	    tmp->clientid=qwi->clientid;
	    tmp->name=QString(qwi->name);
	    ret->append(tmp);
	}
	delete qin;
    }
    return ret;
}

#ifndef QT_NO_QWS_TRANSFORMED
extern void qws_setScreenTransformation( int );
extern void qws_mapPixmaps( bool from );
extern void qws_clearLoadedFonts();
#endif

void QWSDisplay::setTransformation( int t )
{
#ifndef QT_NO_QWS_TRANSFORMED
    QRect mwr = qt_screen->mapToDevice(qt_maxWindowRect,
	QSize(qt_screen->width(), qt_screen->height()) );

    QPixmapCache::clear();
    qws_clearLoadedFonts();
    qws_mapPixmaps( TRUE );
    qws_setScreenTransformation( t );
    qws_mapPixmaps( FALSE );

    if ( qt_fbdpy->d->directServerConnection() ) {
	qwsServer->resetGfx();
	qwsServer->refresh();
    }

    QSize olds = qApp->desktop()->size();
    qApp->desktop()->resize( qt_screen->width(), qt_screen->height() );
    qApp->postEvent( qApp->desktop(), new QResizeEvent(qApp->desktop()->size(), olds) );

    QWidgetList  *list = QApplication::topLevelWidgets();
    if ( list ) {
	QWidgetListIt it( *list );
	QWidget * w;
	while ( (w=it.current()) != 0 ) {
	    ++it;
	    if ( !w->testWFlags(Qt::WType_Desktop) ) {
		QETWidget *etw = (QETWidget*)w;
		if (etw->isFullScreen())
		    etw->setGeometry(qApp->desktop()->rect());
		else
		    etw->updateRegion();
		if ( etw->isVisible() ) {
		    etw->repaintHierarchy( etw->geometry(), TRUE );
		    etw->repaintDecoration( qApp->desktop()->rect(), TRUE );
		}
	    }
	}
	delete list;
    }

    // only update the mwr if it is full screen.
    if ( mwr == QRect(0, 0, qt_screen->deviceWidth(), qt_screen->deviceHeight()) )
	qt_setMaxWindowRect( mwr );
#endif
}

static bool	qt_try_modal( QWidget *, QWSEvent * );

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();

#ifndef QT_NO_PALETTE
// Palette handling
extern QPalette *qt_std_pal;
extern void qt_create_std_palette();
#endif

/*****************************************************************************
  qt_init() - initializes Qt/FB
 *****************************************************************************/



static void qt_set_qws_resources()

{
#ifndef QT_NO_PALETTE
    if ( !qt_std_pal )
	qt_create_std_palette();
    if ( appFont )
	QApplication::setFont( QFont(appFont) );

    if ( appBGCol || appBTNCol || appFGCol ) {
    	QColor btn;
	QColor bg;
	QColor fg;
	if ( appBGCol )
	    bg = QColor( appBGCol );
	else
	    bg = qt_std_pal->active().background();
	if ( appFGCol )
	    fg = QColor( appFGCol );
	else
	    fg = qt_std_pal->active().foreground();
	if ( appBTNCol )
	    btn = QColor( appBTNCol);
	else
	    btn = qt_std_pal->active().button();
	int h,s,v;
	fg.hsv(&h,&s,&v);
	QColor base = Qt::white;
	bool bright_mode = FALSE;
	if (v >= 255-50) {
	    base = btn.dark(150);
	    bright_mode = TRUE;
	}

	QColorGroup cg( fg, btn, btn.light(),
			btn.dark(), btn.dark(150), fg, Qt::white, base, bg );
	if (bright_mode) {
	    cg.setColor( QColorGroup::HighlightedText, base );
	    cg.setColor( QColorGroup::Highlight, Qt::white );
	}
	else {
	    cg.setColor( QColorGroup::HighlightedText, Qt::white );
	    cg.setColor( QColorGroup::Highlight, Qt::darkBlue );
	}
	QColor disabled( (fg.red()+btn.red())/2,
			 (fg.green()+btn.green())/2,
			 (fg.blue()+btn.blue())/2);
	QColorGroup dcg( disabled, btn, btn.light( 125 ), btn.dark(), btn.dark(150),
			 disabled, Qt::white, Qt::white, bg );
	QPalette pal( cg, dcg, cg );
	if ( pal != *qt_std_pal && pal != QApplication::palette() )
	    QApplication::setPalette( pal, TRUE );
	*qt_std_pal = pal;
    }
#endif // QT_NO_PALETTE
}

static void init_display()
{
    if ( qt_fbdpy ) return; // workaround server==client case

    setlocale( LC_ALL, "" );		// use correct char set mapping
    setlocale( LC_NUMERIC, "C" );	// make sprintf()/scanf() work

    // Connect to FB server

    qt_fbdpy = new QWSDisplay();

    // Get display parameters

    // Set paintdevice parameters

    // XXX initial info sent from server

    // Misc. initialization

    QColor::initialize();
    QFont::initialize();
#ifndef QT_NO_CURSOR
    QCursor::initialize();
#endif
    QPainter::initialize();
    QFontManager::initialize();
#ifndef QT_NO_QWS_MANAGER
    qws_decoration = QWSManager::newDefaultDecoration();
#endif

    incoming.setAutoDelete(true);
    outgoing.setAutoDelete(true);

    qApp->setName( appName );

    QFont f;
    f = QFont( "helvetica", 10 );
    QApplication::setFont( f );

    qt_set_qws_resources();

}

void qt_init_display()
{
    qt_is_gui_used = TRUE;
    qws_single_process = TRUE;
    init_display();
}

static bool read_bool_env_var( const char *var, bool defaultvalue )
{
    // returns TRUE if env variable is set to non-zero
    // returns FALSE if env var is set to zero
    // returns defaultvalue if env var not set
    char *x = getenv(var);
    return (x && *x) ? (strcmp(x,"0") != 0) : defaultvalue;
}

void qt_init( int *argcptr, char **argv, QApplication::Type type )
{
    if ( type == QApplication::GuiServer )
	qt_is_gui_used = FALSE; //we'll turn it on in a second
    qws_sw_cursor = read_bool_env_var("QWS_SW_CURSOR",qws_sw_cursor);
    qws_screen_is_interlaced = read_bool_env_var("QWS_INTERLACE",FALSE);

    const char *display = getenv("QWS_DISPLAY");
    if ( display )
	qws_display_spec = strdup(display); // since we setenv later!

    //qws_savefonts = getenv("QWS_SAVEFONTS") != 0;
    //qws_shared_memory = getenv("QWS_NOSHARED") == 0;

    int flags = 0;
    char *p;
    int argc = *argcptr;
    int j;

    // Set application name

    p = strrchr( argv[0], '/' );
    appName = p ? p + 1 : argv[0];

    // Get command line params

    j = 1;
    for ( int i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
	if ( arg == "-fn" || arg == "-font" ) {
	    if ( ++i < argc )
		appFont = argv[i];
	} else if ( arg == "-bg" || arg == "-background" ) {
	    if ( ++i < argc )
		appBGCol = argv[i];
	} else if ( arg == "-btn" || arg == "-button" ) {
	    if ( ++i < argc )
		appBTNCol = argv[i];
	} else if ( arg == "-fg" || arg == "-foreground" ) {
	    if ( ++i < argc )
		appFGCol = argv[i];
	} else if ( arg == "-name" ) {
	    if ( ++i < argc )
		appName = argv[i];
	} else if ( arg == "-title" ) {
	    if ( ++i < argc )
		mwTitle = argv[i];
	} else if ( arg == "-geometry" ) {
	    if ( ++i < argc )
		mwGeometry = argv[i];
	} else if ( arg == "-shared" ) {
	    qws_shared_memory = TRUE;
	} else if ( arg == "-noshared" ) {
	    qws_shared_memory = FALSE;
	} else if ( arg == "-savefonts" ) {
	    qws_savefonts = TRUE;
	} else if ( arg == "-nosavefonts" ) {
	    qws_savefonts = FALSE;
	} else if ( arg == "-swcursor" ) {
	    qws_sw_cursor = TRUE;
	} else if ( arg == "-noswcursor" ) {
	    qws_sw_cursor = FALSE;
	} else if ( arg == "-keyboard" ) {
	    flags &= ~QWSServer::DisableKeyboard;
	} else if ( arg == "-nokeyboard" ) {
	    flags |= QWSServer::DisableKeyboard;
	} else if ( arg == "-mouse" ) {
	    flags &= ~QWSServer::DisableMouse;
	} else if ( arg == "-nomouse" ) {
	    flags |= QWSServer::DisableMouse;
	} else if ( arg == "-qws" ) {
	    type = QApplication::GuiServer;
	} else if ( arg == "-interlaced" ) {
	    qws_screen_is_interlaced = TRUE;
	} else if ( arg == "-display" ) {
	    if ( ++i < argc )
		qws_display_spec = argv[i];
	} else {
	    argv[j++] = argv[i];
	}
    }

    *argcptr = j;

    gettimeofday( &watchtime, 0 );
    mouseInWidget = new QGuardedPtr<QWidget>;

    QRegExp r( ":[0-9]+" );  // supports more than 10 displays
    int len;
    int m = r.match( QString(qws_display_spec) , 0, &len );
    if ( m >= 0 ) {
	QString num = QString(qws_display_spec).mid( m+1, len-1 );
	qws_display_id = num.toInt();
    }

    if ( type == QApplication::GuiServer ) {
	qt_appType = type;
	qws_single_process = TRUE;
	QWSServer::startup(flags);
	setenv("QWS_DISPLAY", qws_display_spec, 0);
    }

    if( qt_is_gui_used )
	init_display();

#ifndef QT_NO_STYLE_INTERLACE
    if ( qws_screen_is_interlaced )
	QApplication::setStyle( new QInterlaceStyle );
#endif


#ifndef QT_NO_NETWORKPROTOCOL
    qInitNetworkProtocols();
#endif

#if defined(_OS_UNIX_) && defined(QT_THREAD_SUPPORT)
    pipe( qt_thread_pipe );
#endif
}

/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    if ( postRList ) {
	QVFuncList::Iterator it = postRList->begin();
	while ( it != postRList->end() ) {	// call post routines
	    (**it)();
	    postRList->remove( it );
	    it = postRList->begin();
	}
	delete postRList;
	postRList = 0;
    }

    cleanupTimers();
    QPixmapCache::clear();
    QPainter::cleanup();
#ifndef QT_NO_CURSOR
    QCursor::cleanup();
#endif
    QFont::cleanup();
    QColor::cleanup();
    QFontManager::cleanup();

    if ( qws_single_process ) {
	QWSServer::closedown();
    }
    if ( qt_is_gui_used ) {
	delete qt_fbdpy;
    }
    qt_fbdpy = 0;

#ifndef QT_NO_QWS_MANAGER
    delete qws_decoration;
#endif

    delete activeBeforePopup;
    activeBeforePopup = 0;
    delete mouseInWidget;
    mouseInWidget = 0;
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

void qAddPostRoutine( Q_CleanUpFunction p )
{
    if ( !postRList ) {
	postRList = new QVFuncList;
	CHECK_PTR( postRList );
    }
    postRList->prepend( p );
}


char *qAppName()				// get application name
{
    return (char*)/* bin-compat */  appName;
}

/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

#define NoValue         0x0000
#define XValue          0x0001
#define YValue          0x0002
#define WidthValue      0x0004
#define HeightValue     0x0008
#define AllValues       0x000F
#define XNegative       0x0010
#define YNegative       0x0020

/* Copyright notice for ReadInteger and parseGeometry

Copyright (c) 1985, 1986, 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/
/*
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged.
 */

static int
ReadInteger(char *string, char **NextString)
{
    register int Result = 0;
    int Sign = 1;

    if (*string == '+')
	string++;
    else if (*string == '-')
    {
	string++;
	Sign = -1;
    }
    for (; (*string >= '0') && (*string <= '9'); string++)
    {
	Result = (Result * 10) + (*string - '0');
    }
    *NextString = string;
    if (Sign >= 0)
	return (Result);
    else
	return (-Result);
}

static int parseGeometry( const char* string,
			  int* x, int* y, int* width, int* height )
{
	int mask = NoValue;
	register char *strind;
	unsigned int tempWidth=0, tempHeight=0;
	int tempX=0, tempY=0;
	char *nextCharacter;

	if ( !string || (*string == '\0')) return(mask);
	if (*string == '=')
		string++;  /* ignore possible '=' at beg of geometry spec */

	strind = (char *)string;
	if (*strind != '+' && *strind != '-' && *strind != 'x') {
		tempWidth = ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter)
		    return (0);
		strind = nextCharacter;
		mask |= WidthValue;
	}

	if (*strind == 'x' || *strind == 'X') {
		strind++;
		tempHeight = ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter)
		    return (0);
		strind = nextCharacter;
		mask |= HeightValue;
	}

	if ((*strind == '+') || (*strind == '-')) {
		if (*strind == '-') {
  			strind++;
			tempX = -ReadInteger(strind, &nextCharacter);
			if (strind == nextCharacter)
			    return (0);
			strind = nextCharacter;
			mask |= XNegative;

		}
		else
		{	strind++;
			tempX = ReadInteger(strind, &nextCharacter);
			if (strind == nextCharacter)
			    return(0);
			strind = nextCharacter;
		}
		mask |= XValue;
		if ((*strind == '+') || (*strind == '-')) {
			if (*strind == '-') {
				strind++;
				tempY = -ReadInteger(strind, &nextCharacter);
				if (strind == nextCharacter)
			    	    return(0);
				strind = nextCharacter;
				mask |= YNegative;

			}
			else
			{
				strind++;
				tempY = ReadInteger(strind, &nextCharacter);
				if (strind == nextCharacter)
			    	    return(0);
				strind = nextCharacter;
			}
			mask |= YValue;
		}
	}

	/* If strind isn't at the end of the string the it's an invalid
		geometry specification. */

	if (*strind != '\0') return (0);

	if (mask & XValue)
	    *x = tempX;
 	if (mask & YValue)
	    *y = tempY;
	if (mask & WidthValue)
            *width = tempWidth;
	if (mask & HeightValue)
            *height = tempHeight;
	return (mask);
}


void QApplication::setMainWidget( QWidget *mainWidget )
{
    main_widget = mainWidget;
    if ( main_widget ) {			// give WM command line
	if ( mwTitle ) {
	    // XXX
	}
	if ( mwGeometry ) {			// parse geometry
	    int x, y;
	    int w, h;
	    int m = parseGeometry( mwGeometry, &x, &y, &w, &h );
	    QSize minSize = main_widget->minimumSize();
	    QSize maxSize = main_widget->maximumSize();
	    if ( (m & XValue) == 0 )
		x = main_widget->geometry().x();
	    if ( (m & YValue) == 0 )
		y = main_widget->geometry().y();
	    if ( (m & WidthValue) == 0 )
		w = main_widget->width();
	    if ( (m & HeightValue) == 0 )
		h = main_widget->height();
	    w = QMIN(w,maxSize.width());
	    h = QMIN(h,maxSize.height());
	    w = QMAX(w,minSize.width());
	    h = QMAX(h,minSize.height());
	    if ( (m & XNegative) ) {
		x = desktop()->width()  + x - w;
		qt_widget_tlw_gravity = 3;
	    }
	    if ( (m & YNegative) ) {
		y = desktop()->height() + y - h;
		qt_widget_tlw_gravity = (m & XNegative) ? 9 : 7;
	    }
	    main_widget->setGeometry( x, y, w, h );
	}
    }
}


/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/
#ifndef QT_NO_CURSOR
typedef QList<QCursor> QCursorList;

static QCursorList *cursorStack = 0;
void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
    if ( !cursorStack ) {
	qws_overrideCursor = TRUE;
	cursorStack = new QCursorList;
	CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    CHECK_PTR( app_cursor );
    if ( replace )
	cursorStack->removeLast();
    cursorStack->append( app_cursor );
    QWidget *w = QWidget::mouseGrabber();
    if ( !w )
	w = widgetAt(*qt_last_x, *qt_last_y, FALSE);
    if ( !w )
	w = desktop();
    QPaintDevice::qwsDisplay()->selectCursor(w, (int)app_cursor->handle());
}

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    QWidget *w = QWidget::mouseGrabber();
    if ( !w )
	w = widgetAt(*qt_last_x, *qt_last_y, FALSE);
    if ( !w )
	w = desktop();
    if ( !app_cursor ) {
	delete cursorStack;
	cursorStack = 0;
	qws_overrideCursor = FALSE;
	if ( w->testWState(WState_OwnCursor) )
	    QPaintDevice::qwsDisplay()->selectCursor(w, (int)w->cursor().handle());
	else
	    QPaintDevice::qwsDisplay()->selectCursor(w, ArrowCursor);
    } else {
	QPaintDevice::qwsDisplay()->selectCursor(w, (int)app_cursor->handle());
    }
}
#endif// QT_NO_CURSOR


void QApplication::setGlobalMouseTracking( bool enable )
{
    bool tellAllWidgets;
    if ( enable ) {
	tellAllWidgets = (++app_tracking == 1);
    } else {
	tellAllWidgets = (--app_tracking == 0);
    }
    if ( tellAllWidgets ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {
	    if ( app_tracking > 0 ) {		// switch on
		if ( !w->testWState(WState_MouseTracking) ) {
		    w->setMouseTracking( TRUE );
		    w->clearWState(WState_MouseTracking);
		}
	    } else {				// switch off
		if ( !w->testWState(WState_MouseTracking) ) {
		    w->setWState(WState_MouseTracking);
		    w->setMouseTracking( FALSE );
		}
	    }
	    ++it;
	}
    }
}


/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

QWidget *QApplication::findWidget( const QObjectList& list,
				   const QPoint &pos, bool rec )
{
    QWidget *w;
    QObjectListIt it( list );
    it.toLast();
    while ( it.current() ) {
	if ( it.current()->isWidgetType() ) {
	    w = (QWidget*)it.current();
	    if ( w->isVisible() && w->geometry().contains(pos)
		 && w->requestedRegion().contains( qt_screen->mapToDevice( w->mapToGlobal(w->mapFromParent(pos)), QSize(qt_screen->width(), qt_screen->height()) ) ) ) {
		if ( !rec )
		    return w;
		QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		return c ? c : w;
	    }
	}
	--it;
    }
    return 0;
}

QWidget *QApplication::findChildWidget( const QWidget *p, const QPoint &pos )
{
    if ( p->children() ) {
	return findWidget( *p->children(), pos, TRUE );
    }
    return 0;
}

QWidget *QApplication::widgetAt( int x, int y, bool child )
{
    // XXX not a fast function...
    QWidgetList *list = topLevelWidgets();

    QPoint pos(x,y);

    if ( list ) {
	QWidget *w;
	QWidgetListIt it( *list );
	it.toLast();
	while ( it.current() ) {
	    w = (QWidget*)it.current();
	    if ( w->isVisible() && w->geometry().contains(pos)
		 && w->allocatedRegion().contains( qt_screen->mapToDevice( w->mapToGlobal(w->mapFromParent(pos)), QSize(qt_screen->width(), qt_screen->height()) ) ) ) {
	        delete list;
		if ( !child )
		    return w;
		QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		return c ? c : w;
	    }
	    --it;
	}
	delete list;
	return 0;
    } else {
	return 0;
    }
}

#ifndef QT_NO_SOUND
QSound *qt_beep_sound = 0;
#endif

void QApplication::beep()
{
#ifndef QT_NO_SOUND
    if ( qt_beep_sound )
	qt_beep_sound->play();
#endif
}



/*****************************************************************************
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocketnotifier.h) provides installable callbacks
  for select() through the internal function qt_set_socket_handler().
 *****************************************************************************/

struct QSockNot {
    QObject *obj;
    int	     fd;
    fd_set  *queue;
};

typedef QList<QSockNot> QSNList;
typedef QListIterator<QSockNot> QSNListIt;

static int	sn_highest = -1;
static QSNList *sn_read	   = 0;
static QSNList *sn_write   = 0;
static QSNList *sn_except  = 0;

static fd_set	sn_readfds;			// fd set for reading
static fd_set	sn_writefds;			// fd set for writing
static fd_set	sn_exceptfds;			// fd set for exceptions
static fd_set	sn_queued_read;
static fd_set	sn_queued_write;
static fd_set	sn_queued_except;

static struct SN_Type {
    QSNList **list;
    fd_set   *fdspec;
    fd_set   *fdres;
    fd_set   *queue;
} sn_vec[3] = {
    { &sn_read,	  &sn_readfds,	 &app_readfds,   &sn_queued_read },
    { &sn_write,  &sn_writefds,	 &app_writefds,  &sn_queued_write },
    { &sn_except, &sn_exceptfds, &app_exceptfds, &sn_queued_except } };


static QSNList *sn_act_list = 0;


static void sn_cleanup()
{
    delete sn_act_list;
    sn_act_list = 0;
    for ( int i=0; i<3; i++ ) {
	delete *sn_vec[i].list;
	*sn_vec[i].list = 0;
    }
}


static void sn_init()
{
    if ( !sn_act_list ) {
	sn_act_list = new QSNList;
	CHECK_PTR( sn_act_list );
	qAddPostRoutine( sn_cleanup );
    }
}


bool qt_set_socket_handler( int sockfd, int type, QObject *obj, bool enable )
{
    if ( sockfd < 0 || type < 0 || type > 2 || obj == 0 ) {
#if defined(CHECK_RANGE)
	qWarning( "QSocketNotifier: Internal error" );
#endif
	return FALSE;
    }

    QSNList  *list = *sn_vec[type].list;
    fd_set   *fds  =  sn_vec[type].fdspec;
    QSockNot *sn;

    if ( enable ) {				// enable notifier
	if ( !list ) {
	    sn_init();
	    list = new QSNList;			// create new list
	    CHECK_PTR( list );
	    list->setAutoDelete( TRUE );
	    *sn_vec[type].list = list;
	    FD_ZERO( fds );
	    FD_ZERO( sn_vec[type].queue );
	}
	sn = new QSockNot;
	CHECK_PTR( sn );
	sn->obj = obj;
	sn->fd	= sockfd;
	sn->queue = sn_vec[type].queue;
	if ( list->isEmpty() ) {
	    list->insert( 0, sn );
	} else {				// sort list by fd, decreasing
	    QSockNot *p = list->first();
	    while ( p && p->fd > sockfd )
		p = list->next();
#if defined(CHECK_STATE)
	    if ( p && p->fd == sockfd ) {
		static const char *t[] = { "read", "write", "exception" };
		qWarning( "QSocketNotifier: Multiple socket notifiers for "
			 "same socket %d and type %s", sockfd, t[type] );
	    }
#endif
	    if ( p )
		list->insert( list->at(), sn );
	    else
		list->append( sn );
	}
	FD_SET( sockfd, fds );
	sn_highest = QMAX(sn_highest,sockfd);

    } else {					// disable notifier

	if ( list == 0 )
	    return FALSE;			// no such fd set
	QSockNot *sn = list->first();
	while ( sn && !(sn->obj == obj && sn->fd == sockfd) )
	    sn = list->next();
	if ( !sn )				// not found
	    return FALSE;
	FD_CLR( sockfd, fds );			// clear fd bit
	FD_CLR( sockfd, sn->queue );
	if ( sn_act_list )
	    sn_act_list->removeRef( sn );	// remove from activation list
	list->remove();				// remove notifier found above
	if ( sn_highest == sockfd ) {		// find highest fd
	    sn_highest = -1;
	    for ( int i=0; i<3; i++ ) {
		if ( *sn_vec[i].list && (*sn_vec[i].list)->count() )
		    sn_highest = QMAX(sn_highest,  // list is fd-sorted
				      (*sn_vec[i].list)->getFirst()->fd);
	    }
	}
    }

    return TRUE;
}


//
// We choose a random activation order to be more fair under high load.
// If a constant order is used and a peer early in the list can
// saturate the IO, it might grab our attention completely.
// Also, if we're using a straight list, the callback routines may
// delete other entries from the list before those other entries are
// processed.
//

static int sn_activate()
{
    if ( !sn_act_list )
	sn_init();
    int i, n_act = 0;
    for ( i=0; i<3; i++ ) {			// for each list...
	if ( *sn_vec[i].list ) {		// any entries?
	    QSNList  *list = *sn_vec[i].list;
	    fd_set   *fds  = sn_vec[i].fdres;
	    QSockNot *sn   = list->first();
	    while ( sn ) {
		if ( FD_ISSET( sn->fd, fds ) &&	// store away for activation
		     !FD_ISSET( sn->fd, sn->queue ) ) {
		    sn_act_list->insert( (rand() & 0xff) %
					 (sn_act_list->count()+1),
					 sn );
		    FD_SET( sn->fd, sn->queue );
		}
		sn = list->next();
	    }
	}
    }
    if ( sn_act_list->count() > 0 ) {		// activate entries
	QEvent event( QEvent::SockAct );
	QSNListIt it( *sn_act_list );
	QSockNot *sn;
	while ( (sn=it.current()) ) {
	    ++it;
	    sn_act_list->removeRef( sn );
	    if ( FD_ISSET(sn->fd, sn->queue) ) {
		FD_CLR( sn->fd, sn->queue );
		QApplication::sendEvent( sn->obj, &event );
		n_act++;
	    }
	}
    }
    return n_act;
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/

int QApplication::exec()
{
    quit_now = FALSE;
    quit_code = 0;

#if defined(QT_THREAD_SUPPORT)
    if (qt_is_gui_used)
	qApp->unlock(FALSE);
#endif

    enter_loop();

    return quit_code;
}


bool QApplication::processNextEvent( bool canWait )
{
    int	   nevents = 0;

#if defined(QT_THREAD_SUPPORT)
    qApp->lock();
#endif

    emit guiThreadAwake();

    if (qt_is_gui_used ) {
	sendPostedEvents();

	while ( qt_fbdpy->eventPending() ) {	// also flushes output buffer
	    if ( app_exit_loop ) {		// quit between events
#if defined(QT_THREAD_SUPPORT)
		    qApp->unlock(FALSE);
#endif
		return FALSE;
	    }
	    QWSEvent *event = qt_fbdpy->getEvent();	// get next event
	    nevents++;

	    bool ret = qwsProcessEvent( event ) == 1;
	    delete event;
	    if ( ret ) {
#if defined(QT_THREAD_SUPPORT)
		    qApp->unlock(FALSE);
#endif
		return TRUE;
	    }
	}
    }
    if ( app_exit_loop ) {			// break immediately
#if defined(QT_THREAD_SUPPORT)
	qApp->unlock(FALSE);
#endif

	return FALSE;
    }

    if ( !outgoing.isEmpty() )
	QWSServer::processEventQueue();

    sendPostedEvents();

    static timeval zerotm;
    timeval *tm = qt_wait_timer();		// wait for timer or X event
    bool qt_postedEventCount();
    if ( !incoming.isEmpty() || qt_postedEventCount() > 0 )
	canWait = FALSE;
    if ( !canWait ) {
	if ( !tm )
	    tm = &zerotm;
	tm->tv_sec  = 0;			// no time to wait
	tm->tv_usec = 0;
    }
    if ( sn_highest >= 0 ) {			// has socket notifier(s)
	if ( sn_read )
	    app_readfds = sn_readfds;
	else
	    FD_ZERO( &app_readfds );
	if ( sn_write )
	    app_writefds = sn_writefds;
	if ( sn_except )
	    app_exceptfds = sn_exceptfds;
    } else {
	FD_ZERO( &app_readfds );
    }

    int nsel;

    int highest=sn_highest;

#if defined(_OS_UNIX_) && defined(QT_THREAD_SUPPORT)
    FD_SET( qt_thread_pipe[0], &app_readfds );
    highest = QMAX( highest, qt_thread_pipe[0] );
#endif

    if ( qt_preselect_handler ) {
	QVFuncList::Iterator end = qt_preselect_handler->end();
	for ( QVFuncList::Iterator it = qt_preselect_handler->begin(); it != end; ++it )
	    (**it)();
    }

#if defined(_OS_WIN32_)
#define FDCAST (fd_set*)
#else
#define FDCAST (void*)
#endif

#if defined(QT_THREAD_SUPPORT)
    qApp->unlock(FALSE);
#endif

    nsel = select( highest + 1,
		   FDCAST (&app_readfds),
		   FDCAST (sn_write  ? &app_writefds  : 0),
		   FDCAST (sn_except ? &app_exceptfds : 0),
		   tm );
#undef FDCAST

#if defined(QT_THREAD_SUPPORT)
    qApp->lock();
#endif

#if defined(_OS_UNIX_)&& defined(QT_THREAD_SUPPORT)
    if ( nsel > 0 && FD_ISSET( qt_thread_pipe[0], &app_readfds ) ) {
	char c;
	::read(qt_thread_pipe[0], &c, 1);
    }
#endif

    if ( qt_postselect_handler ) {
	QVFuncList::Iterator end = qt_postselect_handler->end();
	for ( QVFuncList::Iterator it = qt_postselect_handler->begin(); it != end; ++it )
	    (**it)();
    }

    if ( nsel == -1 ) {
	if ( errno == EINTR || errno == EAGAIN ) {
	    errno = 0;

#if defined(QT_THREAD_SUPPORT)
	    qApp->unlock(FALSE);
#endif

	    return (nevents > 0);
	} else {
	    // select error
	}
    } else if ( nsel > 0 && sn_highest >= 0 ) {
	nevents += sn_activate();
    }

    nevents += qt_activate_timers();		// activate timers

#if defined(QT_THREAD_SUPPORT)
    qApp->unlock(FALSE);
#endif

    return (nevents > 0);
}

int QApplication::qwsProcessEvent( QWSEvent* event )
{
    if ( qwsEventFilter(event) )			// send through app filter
	return 1;
#ifndef QT_NO_QWS_PROPERTIES
    if ( event->type == QWSEvent::PropertyNotify ) {
	QWSPropertyNotifyEvent *e = (QWSPropertyNotifyEvent*)event;
	if ( e->simpleData.property == 424242 ) {	// Clipboard
	    if ( qt_clipboard ) {
		QCustomEvent e( QEvent::Clipboard, event );
		QApplication::sendEvent( qt_clipboard, &e );
	    }
	}
    } else if ( event->type == QWSEvent::PropertyReply ) {
	QWSPropertyReplyEvent *e = (QWSPropertyReplyEvent*)event;
	int len = e->simpleData.len;
	char *data;
	if ( len <= 0 ) {
	    data = 0;
	} else {
	    data = new char[len];
	    memcpy( data, e->data, len ) ;
	}
	QPaintDevice::qwsDisplay()->getPropertyLen = len;
	QPaintDevice::qwsDisplay()->getPropertyData = data;
    }
#endif //QT_NO_QWS_PROPERTIES
#ifndef QT_NO_COP
    if ( event->type == QWSEvent::QCopMessage ) {
	QWSQCopMessageEvent *e = (QWSQCopMessageEvent*)event;
	QCopChannel::sendLocally( e->channel, e->message, e->data );
	return 0;
    }
#endif

    QETWidget *widget = (QETWidget*)QWidget::find( (WId)event->window() );

    QETWidget *keywidget=0;
    bool grabbed=FALSE;
    if ( event->type==QWSEvent::Key || event->type == QWSEvent::IMEvent ) {
	keywidget = (QETWidget*)QWidget::keyboardGrabber();
	if ( keywidget ) {
	    grabbed = TRUE;
	} else {
	    if ( focus_widget && focus_widget->isVisible() )
		keywidget = (QETWidget*)focus_widget;
	    else if ( widget )
		keywidget = (QETWidget*)widget->topLevelWidget();
	}
    } else if ( event->type==QWSEvent::MaxWindowRect ) {
	QRect r = ((QWSMaxWindowRectEvent*)event)->simpleData.rect;
	extern void qt_setMaxWindowRect(const QRect& r);
	qt_setMaxWindowRect(r);
	return 0;
    } else if ( widget && event->type==QWSEvent::Mouse ) {
	// The mouse event is to one of my top-level widgets
	// which one?
	const int btnMask = LeftButton | RightButton | MidButton;
	QPoint p(event->asMouse()->simpleData.x_root,
		 event->asMouse()->simpleData.y_root);
	int mouseButtonState = event->asMouse()->simpleData.state & btnMask;
	static int btnstate = 0;

	QETWidget *w = (QETWidget*)QWidget::mouseGrabber();
	if ( w && !mouseButtonState && qt_pressGrab == w )
	    qt_pressGrab = 0;
#ifndef QT_NO_QWS_MANAGER
	if ( !w )
	    w = (QETWidget*)QWSManager::grabbedMouse();
#endif
	if (w) {
	    // Our mouse is grabbed - send it.
	    widget = w;
	    btnstate = mouseButtonState;
	} else {
	    static QWidget *gw = 0;
	    // Three jobs to do here:
	    // 1. find the child widget this event belongs to.
	    // 2. make sure the cursor is correct.
	    // 3. handle implicit mouse grab due to button press.
	    w = widget; // w is the widget the cursor is in.
	    QSize s( qt_screen->width(), qt_screen->height() );
	    QPoint dp = qt_screen->mapToDevice( p, s );
	    if ( widget->alloc_region.contains(dp) ) {
		// Find the child widget that the cursor is in.
		w = (QETWidget*)findChildWidget(widget, widget->mapFromParent(p));
		w = w ? (QETWidget*)w : widget;
#ifndef QT_NO_CURSOR
		// Update Cursor.
		if ( !gw || gw != w || qt_last_cursor == 0xffffffff ) {
		    QCursor *curs = app_cursor;
		    if (!curs && w->extraData()) {
			curs = w->extraData()->curs;
		    }
		    QWidget *pw = w;
		    // If this widget has no cursor set, try parent.
		    while (!curs) {
			pw = pw->parentWidget();
			if (!pw)
			    break;
			if (pw->extraData())
			    curs = pw->extraData()->curs;
		    }
		    if ( !qws_overrideCursor ) {    // is override cursor active?
			if (curs)
			    QPaintDevice::qwsDisplay()->selectCursor(widget, (int)curs->handle());
			else
			    QPaintDevice::qwsDisplay()->selectCursor(widget, ArrowCursor);
		    }
		}
#endif
		gw = w;
	    } else {
		// This event is not for any of our widgets
		gw = 0;
	    }
	    if ( mouseButtonState && !btnstate ) {
		// The server has grabbed the mouse for us.
		// Remember which of my widgets has it.
		qt_pressGrab = w;
		if ( !widget->isActiveWindow() &&
		     ( !app_do_modal || QApplication::activeModalWidget() == widget ) &&
		     !widget->testWFlags(WStyle_NoBorder|WStyle_Tool) ) {
		    widget->setActiveWindow();
		    if ( widget->raiseOnClick() )
			widget->raise();
		}
	    }
	    btnstate = mouseButtonState;
	    widget = w;
	}
    }

    if ( !widget ) {				// don't know this window
	if ( !QWidget::mouseGrabber()
#ifndef QT_NO_QWS_MANAGER
	    && !QWSManager::grabbedMouse() 
#endif
	    ) {
	    qt_last_cursor = 0xffffffff; // cursor can be changed by another application
	}

#ifndef QT_KEYPAD_MODE
	
	QWidget* popup = QApplication::activePopupWidget();

	// ### This really needs to be looked at closely.   Don't think we want it.
	if ( popup ) {

	    /*
	      That is more than suboptimal. The real solution should
	      do some keyevent and buttonevent translation, so that
	      the popup still continues to work as the user expects.
	      Unfortunately this translation is currently only
	      possible with a known widget. I'll change that soon
	      (Matthias).
	     */

	    // Danger - make sure we don't lock the server
	    switch ( event->type ) {
	    case QWSEvent::Mouse:
	    case QWSEvent::Key:
		do {
		    popup->close();
		} while ( (popup = qApp->activePopupWidget()) );
		return 1;
	    }
	}
#endif
	if ( *mouseInWidget ) {
	    QEvent leave( QEvent::Leave );
	    QApplication::sendEvent( *mouseInWidget, &leave );
	    (*mouseInWidget) = 0;
	}
	return -1;
    }

    if ( app_do_modal )				// modal event handling
	if ( !qt_try_modal(widget, event) ) {
	    return 1;
	}

    if ( widget->qwsEvent(event) )		// send through widget filter
	return 1;

    switch ( event->type ) {

    case QWSEvent::Mouse:			// mouse event
	widget->translateMouseEvent( event->asMouse() );
	break;

    case QWSEvent::Key:				// keyboard event
	if ( keywidget ) // should always exist
	    keywidget->translateKeyEvent( (QWSKeyEvent*)event, grabbed );
	break;

#ifndef QT_NO_QWS_IM
    case QWSEvent::IMEvent:
	if ( keywidget ) // should always exist
	    QInputContext::translateIMEvent( (QWSIMEvent*)event, keywidget );
	break;
#endif

    case QWSEvent::RegionModified:
	widget->translateRegionModifiedEvent( (QWSRegionModifiedEvent*)event );
	break;

    case QWSEvent::Focus:
	if ( ((QWSFocusEvent*)event)->simpleData.get_focus ) {
	    if ( widget == desktop() )
		return TRUE; // not interesting
	    if ( inPopupMode() ) // some delayed focus event to ignore
		break;
	    if (activeWindow() != widget) {
		setActiveWindow(widget);
		((QETWidget *)active_window)->repaintDecoration(desktop()->rect(), FALSE);
		QWidget *w = widget->focusWidget();
		while ( w && w->focusProxy() )
		    w = w->focusProxy();
		if ( w && w->isFocusEnabled() )
		    w->setFocus();
		else
		    widget->focusNextPrevChild( TRUE );
		if ( !focus_widget ) {
		    if ( widget->focusWidget() )
			widget->focusWidget()->setFocus();
		    else
			widget->topLevelWidget()->setFocus();
		}
	    }
	} else {	// lost focus
	    if ( widget == desktop() || !active_window || widget != active_window)
		return TRUE; // not interesting
	    if (widget->topData()->fullscreen) {
		bool endFullScreen = TRUE;
		if ( widget->children() ) {
		    QObjectListIt it(*widget->children());
		    QObject *obj;
		    while ( (obj=it.current()) ) {
			++it;
			if ( obj->isWidgetType() ) {
			    QETWidget* w = (QETWidget*)obj;
			    if ( w->isVisible() && w->topData()->fullscreen
				&& w->isActiveWindow()) {
				endFullScreen = FALSE;
				break;
			    }
			}
		    }
		}
		if (endFullScreen)
		    widget->hide();
	    }
	    if ( focus_widget && !inPopupMode() ) {
		QETWidget *old = (QETWidget *)active_window;
		setActiveWindow(0);
		qt_last_cursor = 0xffffffff;
		if (old)
		    old->repaintDecoration(desktop()->rect(), FALSE);
	    }
	}
	break;

    case QWSEvent::WindowOperation:
	if ( widget == desktop() )
	    return TRUE;
	switch ( ((QWSWindowOperationEvent *)event)->simpleData.op ) {
	    case QWSWindowOperationEvent::Show:
		widget->show();
		break;
	    case QWSWindowOperationEvent::Hide:
		widget->hide();
		break;
	    case QWSWindowOperationEvent::ShowMaximized:
		widget->showMaximized();
		break;
	    case QWSWindowOperationEvent::ShowMinimized:
		widget->showMinimized();
		break;
	    case QWSWindowOperationEvent::ShowNormal:
		widget->showNormal();
		break;
	    case QWSWindowOperationEvent::Close:
		widget->close();
		break;
	}
	break;
    default:
	break;
    }

    return 0;
}

void QApplication::processEvents( int maxtime )
{
    QTime start = QTime::currentTime();
    QTime now;
    while ( !app_exit_loop && processNextEvent(FALSE) ) {
	now = QTime::currentTime();
	if ( start.msecsTo(now) > maxtime )
	    break;
    }
}


/*!
  This virtual function is only implemented under Qt/Embedded.

  If you create an application that inherits QApplication and
  reimplement this function, you get direct access to all QWS
  (Q Window System) events that the are received from the QWS
  master process.

  Return TRUE if you want to stop the event from being processed, or
  return FALSE for normal event dispatching.
*/
bool QApplication::qwsEventFilter( QWSEvent * )
{
    return FALSE;
}

/*!
  Set Qt/Embedded custom color table.

  Qt/Embedded on 8-bpp displays allocates a standard 216 color cube.
  The remaining 40 colors may be used by setting a custom color table in
  the QWS master process before any clients connect.

  \a colorTable is an array of up to 40 custom colors.  \a start is the
  starting index (0-39) and \a numColors is the number of colors to be
  set (1-40).

  This method is non-portable.  It is available \e only in Qt/Embedded.
*/
void QApplication::qwsSetCustomColors( QRgb *colorTable, int start, int numColors )
{
    if ( start < 0 || start > 39 ) {
	qWarning( "QApplication::qwsSetCustomColors - start < 0 || start > 39" );
	return;
    }
    if ( start + numColors > 40 ) {
	numColors = 40 - start;
	qWarning( "QApplication::qwsSetCustomColors - too many colors" );
    }
    start += 216;
    for ( int i = 0; i < numColors; i++ ) {
	qt_screen->set( start + i, qRed(colorTable[i]), qGreen(colorTable[i]),
			qBlue(colorTable[i]) );
    }
}

#ifndef QT_NO_QWS_MANAGER
/*!
  Return the QWSDecoration used for decorating windows.

  This method is non-portable.  It is available \e only in Qt/Embedded.

  \sa QWSDecoration
*/
QWSDecoration &QApplication::qwsDecoration()
{
    return *qws_decoration;
}

/*!
  Set the QWSDecoration derived class to use for decorating the Qt/Embedded
  windows.

  This method is non-portable.  It is available \e only in Qt/Embedded.

  \sa QWSDecoration
*/
void QApplication::qwsSetDecoration( QWSDecoration *d )
{
    if ( d ) {
	delete qws_decoration;
	qws_decoration = d;
	QWidgetList *widgets = topLevelWidgets();
	QWidgetListIt it( *widgets );
	QWidget *w;
	while ( (w=it.current()) != 0 ) {
	    ++it;
	    if ( w->isVisible() && w != desktop() ) {
		((QETWidget *)w)->updateRegion();
		((QETWidget *)w)->repaintDecoration(desktop()->rect(), FALSE);
		if ( w->isMaximized() )
		    w->showMaximized();
	    }
	}
	delete widgets;
    }
}
#endif

bool qt_modal_state()
{
    return app_do_modal;
}

void qt_enter_modal( QWidget *widget )
{
    if ( !qt_modal_stack ) {			// create modal stack
	qt_modal_stack = new QWidgetList;
	CHECK_PTR( qt_modal_stack );
    }
    qt_modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
}


void qt_leave_modal( QWidget *widget )
{
    if ( qt_modal_stack && qt_modal_stack->removeRef(widget) ) {
	if ( qt_modal_stack->isEmpty() ) {
	    delete qt_modal_stack;
	    qt_modal_stack = 0;
	}
    }
    app_do_modal = qt_modal_stack != 0;
}


static bool qt_try_modal( QWidget *widget, QWSEvent *event )
{
    if ( qApp->activePopupWidget() )
	return TRUE;
    // a bit of a hack: use WStyle_Tool as a general ignore-modality
    // flag, also for complex widgets with children.
    if ( widget->testWFlags(Qt::WStyle_Tool) )	// allow tool windows
	return TRUE;

    QWidget *modal=0, *top=QApplication::activeModalWidget();

    QWidget* groupLeader = widget;
    widget = widget->topLevelWidget();

    if ( widget->testWFlags(Qt::WType_Modal) )	// widget is modal
	modal = widget;
    if ( !top || modal == top )			// don't block event
	return TRUE;

    while ( groupLeader && !groupLeader->testWFlags( Qt::WGroupLeader ) )
	groupLeader = groupLeader->parentWidget();

    if ( groupLeader ) {
	// Does groupLeader have a child in qt_modal_stack?
	bool unrelated = TRUE;
	modal = qt_modal_stack->first();
	while (modal && unrelated) {
	    QWidget* p = modal->parentWidget();
	    while ( p && p != groupLeader && !p->testWFlags( Qt::WGroupLeader) ) {
		p = p->parentWidget();
	    }
	    modal = qt_modal_stack->next();
	    if ( p == groupLeader ) unrelated = FALSE;
	}

	if ( unrelated )
	    return TRUE;		// don't block event
    }

    bool block_event  = FALSE;
    bool paint_event = FALSE;

    switch ( event->type ) {
	case QWSEvent::Focus:
	    if ( !((QWSFocusEvent*)event)->simpleData.get_focus )
		break;
	    // drop through
	case QWSEvent::Mouse:			// disallow mouse/key events
	case QWSEvent::Key:
	    block_event	 = TRUE;
	    break;
	case QWSEvent::RegionModified:
	    paint_event = TRUE;
	    break;
    }

    if ( top->parentWidget() == 0 && (block_event || paint_event) )
	top->raise();

    return !block_event;
}


void QApplication::openPopup( QWidget *popup )
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	CHECK_PTR( popupWidgets );
	if ( !activeBeforePopup )
	    activeBeforePopup = new QGuardedPtr<QWidget>;
	(*activeBeforePopup) = active_window;
#ifdef QT_KEYPAD_MODE
	if( qt_modalEditingEnabled ) {
	    if ( active_window && active_window->focusWidget() )
		editingBeforePopup = active_window->focusWidget()->isModalEditing();
	}
#endif
	if (!active_window) {
	    // We have no active TLW.  Tell the window system that we want
	    // focus.
	    QPaintDevice::qwsDisplay()->requestFocus(popup->winId(), TRUE);
	}

	/* only grab if you are the first/parent popup */
	QPaintDevice::qwsDisplay()->grabMouse(popup,TRUE);
	QPaintDevice::qwsDisplay()->grabKeyboard(popup,TRUE);
	popupGrabOk = TRUE;
    }
    popupWidgets->append( popup );		// add to end of list

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    QFocusEvent::setReason( QFocusEvent::Popup );
    active_window = popup;
    if (active_window->focusWidget())
	active_window->focusWidget()->setFocus();
    else
	active_window->setFocus();
    QFocusEvent::resetReason();
}

void QApplication::closePopup( QWidget *popup )
{
    if ( !popupWidgets )
	return;

    popupWidgets->removeRef( popup );
    if (popup == popupOfPopupButtonFocus) {
	popupButtonFocus = 0;
	popupOfPopupButtonFocus = 0;
    }
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( popupGrabOk ) {	// grabbing not disabled
	    QPaintDevice::qwsDisplay()->grabMouse(popup,FALSE);
	    QPaintDevice::qwsDisplay()->grabKeyboard(popup,FALSE);
	    popupGrabOk = FALSE;
	    // XXX ungrab keyboard
	}
	active_window = (*activeBeforePopup);
	// restore the former active window immediately, although
	// we'll get a focusIn later
	if ( active_window ) {
	    QFocusEvent::setReason( QFocusEvent::Popup );
	    if (active_window->focusWidget())
		active_window->focusWidget()->setFocus();
	    else
		active_window->setFocus();
	    QFocusEvent::resetReason();
#ifdef QT_KEYPAD_MODE
	    if( qt_modalEditingEnabled ) {
		if ( editingBeforePopup && active_window->focusWidget())
		    active_window->focusWidget()->setModalEditing(TRUE);
	    }
#endif
	}
    }
     else {
	// popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	 QFocusEvent::setReason( QFocusEvent::Popup );
	 active_window = popupWidgets->getLast();
	 if (active_window->focusWidget())
	     active_window->focusWidget()->setFocus();
	 else
	     active_window->setFocus();
	 QFocusEvent::resetReason();
     }
}


//
// Internal data structure for timers
//

struct TimerInfo {				// internal timer info
    int	     id;				// - timer identifier
    timeval  interval;				// - timer interval
    timeval  timeout;				// - when to sent event
    QObject *obj;				// - object to receive event
};

typedef QList<TimerInfo> TimerList;	// list of TimerInfo structs

static QBitArray *timerBitVec;			// timer bit vector
static TimerList *timerList	= 0;		// timer list


//
// Internal operator functions for timevals
//

static inline bool operator<( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec < t2.tv_sec ||
	  (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

static inline timeval &operator+=( timeval &t1, const timeval &t2 )
{
    t1.tv_sec += t2.tv_sec;
    if ( (t1.tv_usec += t2.tv_usec) >= 1000000 ) {
	t1.tv_sec++;
	t1.tv_usec -= 1000000;
    }
    return t1;
}

static inline timeval operator+( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec + t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec + t2.tv_usec) >= 1000000 ) {
	tmp.tv_sec++;
	tmp.tv_usec -= 1000000;
    }
    return tmp;
}

static inline timeval operator-( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec - t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec - t2.tv_usec) < 0 ) {
	tmp.tv_sec--;
	tmp.tv_usec += 1000000;
    }
    return tmp;
}


//
// Internal functions for manipulating timer data structures.
// The timerBitVec array is used for keeping track of timer identifiers.
//

static int allocTimerId()			// find avail timer identifier
{
    int i = timerBitVec->size()-1;
    while ( i >= 0 && (*timerBitVec)[i] )
	i--;
    if ( i < 0 ) {
	i = timerBitVec->size();
	timerBitVec->resize( 4 * i );
	for( int j=timerBitVec->size()-1; j > i; j-- )
	    timerBitVec->clearBit( j );
    }
    timerBitVec->setBit( i );
    return i+1;
}

static void insertTimer( const TimerInfo *ti )	// insert timer info into list
{
    TimerInfo *t = timerList->first();
    int index = 0;
    while ( t && t->timeout < ti->timeout ) {	// list is sorted by timeout
	t = timerList->next();
	index++;
    }
    timerList->insert( index, ti );		// inserts sorted
}

static inline void getTime( timeval &t )	// get time of day
{
    gettimeofday( &t, 0 );
    while ( t.tv_usec >= 1000000 ) {		// NTP-related fix
	t.tv_usec -= 1000000;
	t.tv_sec++;
    }
    while ( t.tv_usec < 0 ) {
	if ( t.tv_sec > 0 ) {
	    t.tv_usec += 1000000;
	    t.tv_sec--;
	} else {
	    t.tv_usec = 0;
	    break;
	}
    }
}

static void repairTimer( const timeval &time )	// repair broken timer
{
    if ( !timerList )				// not initialized
	return;
    timeval diff = watchtime - time;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// repair all timers
	t->timeout = t->timeout - diff;
	t = timerList->next();
    }
}


//
// Timer activation functions (called from the event loop)
//

bool qt_disable_lowpriority_timers=FALSE;

/*
  Returns the time to wait for the next timer, or null if no timers are
  waiting.

  The result is bounded to qt_wait_timer_max if this exists.
*/

timeval *qt_wait_timer()
{
    static timeval tm;
    bool first = TRUE;
    timeval currentTime;
    if ( !qt_disable_lowpriority_timers && timerList && timerList->count() )
	// there are waiting timers
    {
	getTime( currentTime );
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	TimerInfo *t = timerList->first();	// first waiting timer
	if ( currentTime < t->timeout ) {	// time to wait
	    tm = t->timeout - currentTime;
	} else {
	    tm.tv_sec  = 0;			// no time to wait
	    tm.tv_usec = 0;
	}
	if ( qt_wait_timer_max && *qt_wait_timer_max < tm )
	    tm = *qt_wait_timer_max;
	return &tm;
    }
    if ( qt_wait_timer_max ) {
	tm = *qt_wait_timer_max;
	return &tm;
    }
    return 0;					// no timers
}

/*
  Activates the timer events that have expired. Returns the number of timers
  (not 0-timer) that were activated.
*/

int qt_activate_timers()
{
    if ( !timerList || !timerList->count() )	// no timers
	return 0;
    bool first = TRUE;
    timeval currentTime;
    int maxcount = timerList->count();
    int n_act = 0;
    register TimerInfo *t;
    while ( maxcount-- ) {			// avoid starvation
	getTime( currentTime );			// get current time
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	t = timerList->first();
	if ( !t || currentTime < t->timeout )	// no timer has expired
	    break;
	timerList->take();			// unlink from list
	t->timeout += t->interval;
	if ( t->timeout < currentTime )
	    t->timeout = currentTime + t->interval;
	insertTimer( t );			// relink timer
	if ( t->interval.tv_usec > 0 || t->interval.tv_sec > 0 )
	    n_act++;
	QTimerEvent e( t->id );
	QApplication::sendEvent( t->obj, &e );	// send event
    }
    return n_act;
}


//
// Timer initialization and cleanup routines
//

static void initTimers()			// initialize timers
{
    timerBitVec = new QBitArray( 128 );
    CHECK_PTR( timerBitVec );
    int i = timerBitVec->size();
    while( i-- > 0 )
	timerBitVec->clearBit( i );
    timerList = new TimerList;
    CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
}

static void cleanupTimers()			// cleanup timer data structure
{
    if ( timerList ) {
	delete timerList;
	timerList = 0;
	delete timerBitVec;
	timerBitVec = 0;
    }
}


//
// Main timer functions for starting and killing timers
//

int qStartTimer( int interval, QObject *obj )
{
    if ( !timerList )				// initialize timer data
	initTimers();
    int id = allocTimerId();			// get free timer id
    if ( id <= 0 ||
	 id > (int)timerBitVec->size() || !obj )// cannot create timer
	return 0;
    timerBitVec->setBit( id-1 );		// set timer active
    TimerInfo *t = new TimerInfo;		// create timer
    CHECK_PTR( t );
    t->id = id;
    t->interval.tv_sec  = interval/1000;
    t->interval.tv_usec = (interval%1000)*1000;
    timeval currentTime;
    getTime( currentTime );
    t->timeout = currentTime + t->interval;
    t->obj = obj;
    insertTimer( t );				// put timer in list
    return id;
}

bool qKillTimer( int id )
{
    register TimerInfo *t;
    if ( !timerList || id <= 0 ||
	 id > (int)timerBitVec->size() || !timerBitVec->testBit( id-1 ) )
	return FALSE;				// not init'd or invalid timer
    t = timerList->first();
    while ( t && t->id != id )			// find timer info in list
	t = timerList->next();
    if ( t ) {					// id found
	timerBitVec->clearBit( id-1 );		// set timer inactive
	return timerList->remove();
    }
    else					// id not found
	return FALSE;
}

bool qKillTimer( QObject *obj )
{
    register TimerInfo *t;
    if ( !timerList )				// not initialized
	return FALSE;
    t = timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
	    timerBitVec->clearBit( t->id-1 );
	    timerList->remove();
	    t = timerList->current();
	} else {
	    t = timerList->next();
	}
    }
    return TRUE;
}


/*****************************************************************************
  Event translation; translates FB events to Qt events
 *****************************************************************************/

//
// Mouse event translation
//
// FB doesn't give mouse double click events, so we generate them by
// comparing window, time and position between two mouse press events.
//

static int translateButtonState( int s )
{
    // No translation required at this time
    return s;
}

// Needed for QCursor::pos

static const int AnyButton = (Qt::LeftButton | Qt::MidButton | Qt::RightButton );


//The logic got a little too complex, so I split the function in
//two. This one translates a QWS event into separate move
//and press/release events
//There is code duplication for now, I don't want to rock the boat
//too much at this stage.
bool QETWidget::translateMouseEvent( const QWSMouseEvent *event )
{
    static int old_x_root = -1;
    static int old_y_root = -1;
    static int old_state = 0;
    const QWSMouseEvent::SimpleData &mouse = event->simpleData;

    //since we compress mouse events with the same state, the
    //press/release will have been at the end of the move

    if ( mouse.x_root != old_x_root || mouse.y_root != old_y_root ) {
	old_x_root = mouse.x_root;
	old_y_root = mouse.y_root;
	QWSMouseEvent move = *event;
	move.simpleData.state = old_state;
	dispatchMouseEvent( &move );
    }

    if ( (mouse.state&AnyButton) != (old_state&AnyButton) ) {
	old_state = mouse.state;
	dispatchMouseEvent( event );
    }
    //Must ensure that mouse release event gets to the widget
    //that got the mouse press
    if ( qt_button_down && ( mouse.state & AnyButton ) == 0 )
	qt_button_down = 0;

    return TRUE;
}

bool QETWidget::dispatchMouseEvent( const QWSMouseEvent *event )
{
    static bool manualGrab = FALSE;
    QPoint pos;
    QPoint globalPos;
    int	   button = 0;
    int	   state;

    static int old_x_root = -1;
    static int old_y_root = -1;
    static int old_state = 0;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;
    const QWSMouseEvent::SimpleData &mouse = event->simpleData;
    pos = mapFromGlobal(QPoint( mouse.x_root, mouse.y_root ));
    if ( qt_last_x ) {
	*qt_last_x=mouse.x_root;
	*qt_last_y=mouse.y_root;
    }
    globalPos.rx() = mouse.x_root;
    globalPos.ry() = mouse.y_root;
    state = translateButtonState( mouse.state );

    QWidget *oldFocus = focusWidget();
    //    for (;;) { // Extract move and press/release from one QWSEvent
	QEvent::Type type = QEvent::None;

	if ( mouse.x_root != old_x_root || mouse.y_root != old_y_root ) {
	    old_x_root = mouse.x_root;
	    old_y_root = mouse.y_root;
	    // mouse move
	    // XXX compress motion events
	    type = QEvent::MouseMove;
	} else if ( 0 ) {
	    type = QEvent::MouseMove;
	    if ( !qt_button_down )
		state = state & ~AnyButton;
	} else if ( (mouse.state&AnyButton) != (old_state&AnyButton) ) {
	    for ( button = LeftButton; !type && button <= MidButton; button<<=1 ) {
		if ( (mouse.state&button) != (old_state&button) ) {
		    // button press or release
		    if ( isEnabled() ) {
			QWidget* w = this;
			while ( w->focusProxy() )
			    w = w->focusProxy();
			if ( w->focusPolicy() & QWidget::ClickFocus ) {
			    QFocusEvent::setReason( QFocusEvent::Mouse);
			    w->setFocus();
			    QFocusEvent::resetReason();
			}
		    }
		    if ( mouse.state&button ) { //button press
			qt_button_down = QApplication::findChildWidget( this, pos );	//magic for masked widgets
			if ( !qt_button_down || !qt_button_down->testWFlags(WMouseNoMask) )
			    qt_button_down = this;
			if ( /*XXX mouseActWindow == this &&*/
			     mouseButtonPressed == button &&
			     (long)mouse.time -(long)mouseButtonPressTime
				   < QApplication::doubleClickInterval() &&
			     QABS(mouse.x_root - mouseXPos) < 5 &&
			     QABS(mouse.y_root - mouseYPos) < 5 ) {
			    type = QEvent::MouseButtonDblClick;
			    mouseButtonPressTime -= 2000;	// no double-click next time
			} else {
			    type = QEvent::MouseButtonPress;
			    mouseButtonPressTime = mouse.time;
			}
			mouseButtonPressed = button; 	// save event params for
			mouseXPos = globalPos.x();		// future double click tests
			mouseYPos = globalPos.y();
		    } else {				// mouse button released
			if ( manualGrab ) {			// release manual grab
			    manualGrab = FALSE;
			    // XXX XUngrabPointer( x11Display(), CurrentTime );
			}

			type = QEvent::MouseButtonRelease;
		    }
		    old_state ^= button;
		}
	    }
	    button >>= 1;
	}
	//XXX mouseActWindow = winId();			// save some event params
	mouseButtonState = state;

	if ( type == 0 )				// event consumed
	    return FALSE; //EXIT in the normal case

	if ( qApp->inPopupMode() ) {			// in popup mode
	    QWidget *popup = qApp->activePopupWidget();
	    if ( popup != this ) {
		QSize s( qt_screen->width(), qt_screen->height() );
		QPoint dp = qt_screen->mapToDevice( globalPos, s );
		if ( testWFlags(WType_Popup) && alloc_region.contains(dp) )
		    popup = this;
		else				// send to last popup
		    pos = popup->mapFromGlobal( globalPos );
	    }
	    bool releaseAfter = FALSE;
	    QWidget *popupChild  = QApplication::findChildWidget( popup, pos );
	    QWidget *popupTarget = popupChild ? popupChild : popup;

	    if (popup != popupOfPopupButtonFocus){
		popupButtonFocus = 0;
		popupOfPopupButtonFocus = 0;
	    }

	    if ( !popupTarget->isEnabled() )
		return FALSE; //EXIT special case

	    switch ( type ) {
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonDblClick:
		    popupButtonFocus = popupChild;
		    popupOfPopupButtonFocus = popup;
		    break;
		case QEvent::MouseButtonRelease:
		    releaseAfter = TRUE;
		    break;
		default:
		    break;				// nothing for mouse move
	    }

	    if ( popupButtonFocus ) {
		QMouseEvent e( type, popupButtonFocus->mapFromGlobal(globalPos),
			       globalPos, button, state );
		QApplication::sendEvent( popupButtonFocus, &e );
		if ( releaseAfter ) {
		    popupButtonFocus = 0;
		    popupOfPopupButtonFocus = 0;
		}
	    } else if ( popupChild ) {
		QMouseEvent e( type, popupChild->mapFromGlobal(globalPos),
			       globalPos, button, state );
		QApplication::sendEvent( popupChild, &e );
	    } else {
		QMouseEvent e( type, pos, globalPos, button, state );
		QApplication::sendEvent( popupChild ? popupChild : popup, &e );
	    }

	    if ( releaseAfter )
		qt_button_down = 0;

	} else { //qApp not in popup mode
	    QWidget *widget = this;
	    QWidget *w = QWidget::mouseGrabber();
	    if ( !w && qt_button_down )
		w = qt_button_down;
	    if ( w && w != this ) {
		widget = w;
		pos = mapToGlobal( pos );
		pos = w->mapFromGlobal( pos );
	    }

	    if ( popupCloseDownMode ) {
		popupCloseDownMode = FALSE;
		if ( testWFlags(WType_Popup) )	// ignore replayed event
		    return TRUE; //EXIT
	    }

	    if ( type == QEvent::MouseButtonRelease &&
		 (state & (~button) & ( LeftButton |
					MidButton |
					RightButton)) == 0 ) {
		qt_button_down = 0;
	    }

	    QMouseEvent e( type, pos, globalPos, button, state );
#ifndef QT_NO_QWS_MANAGER
	    if (widget->isTopLevel() && widget->topData()->qwsManager
		&& (widget->topData()->qwsManager->region().contains(globalPos)
		    || (QWSManager::grabbedMouse() && QWidget::mouseGrabber())) ) {
		if ( (*mouseInWidget) ) {
		    QEvent leave( QEvent::Leave );
		    QApplication::sendEvent( *mouseInWidget, &leave );
		    (*mouseInWidget) = 0;
		}
		QApplication::sendEvent( widget->topData()->qwsManager, &e );
	    } else
#endif
		{
		if ( widget != (*mouseInWidget) ) {
		    if ( *mouseInWidget ) {
			QEvent leave( QEvent::Leave );
			QApplication::sendEvent( *mouseInWidget, &leave );
		    }
		    QEvent enter( QEvent::Enter );
		    QApplication::sendEvent( widget, &enter );
		    (*mouseInWidget) = widget;
		}

		QApplication::sendEvent( widget, &e );
	    }
	}
	// }
    return TRUE;
}


bool QETWidget::translateKeyEvent( const QWSKeyEvent *event, bool grab )
{
    int	   code = -1;
    int	   state = event->simpleData.modifiers;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    if ( !isEnabled() )
	return TRUE;

    QEvent::Type type = event->simpleData.is_press ?
	QEvent::KeyPress : QEvent::KeyRelease;
    bool    autor = event->simpleData.is_auto_repeat;
    QString text;
    char   ascii = 0;
    if ( event->simpleData.unicode ) {
	QChar ch(event->simpleData.unicode );
	if ( ch.unicode() != 0xffff )
	    text += ch;
	ascii = ch.latin1();
    }
    code = event->simpleData.keycode;

    bool isAccel = FALSE;
    if (!grab) { // test for accel if the keyboard is not grabbed
	QKeyEvent a( QEvent::AccelAvailable, code, ascii, state, text, FALSE,
		     int(text.length()) );
	a.ignore();
	QApplication::sendEvent( topLevelWidget(), &a );
	isAccel = a.isAccepted();
    }

    if ( !isAccel && !text.isEmpty() && testWState(WState_CompressKeys) ) {
	// the widget wants key compression so it gets it

	// XXX not implemented
    }


    // process accelerates before popups
    QKeyEvent e( type, code, ascii, state, text, autor,
		 int(text.length()) );
    if ( type == QEvent::KeyPress && !grab ) {
	// send accel event to tlw if the keyboard is not grabbed
	QKeyEvent a( QEvent::Accel, code, ascii, state, text, autor,
		     int(text.length()) );
	a.ignore();
	QApplication::sendEvent( topLevelWidget(), &a );
	if ( a.isAccepted() )
	    return TRUE;
    }
    return QApplication::sendEvent( this, &e );
}

void QETWidget::repaintHierarchy(QRegion r, bool post)
{
    r &= geometry();
    if (r.isEmpty())
	return;
    r.translate(-crect.x(),-crect.y());

    if ( post ) {
	QApplication::postEvent(this,new QPaintEvent(r,
		    !testWFlags(QWidget::WRepaintNoErase) ) );
    } else {
	erase(r);

	QPaintEvent e( r );
	setWState( WState_InPaintEvent );
	qt_set_paintevent_clipping( this, r);
	QApplication::sendEvent( this, &e );
	qt_clear_paintevent_clipping();
	clearWState( WState_InPaintEvent );
    }

    if ( children() ) {
	QObjectListIt it(*children());
	register QObject *obj;
	while ( (obj=it.current()) ) {
	    ++it;
	    if ( obj->isWidgetType() ) {
		QETWidget* w = (QETWidget*)obj;
		if ( w->isVisible() )
		    w->repaintHierarchy(r, post);
	    }
	}
    }
}

void QETWidget::repaintDecoration(QRegion r, bool post)
{
#ifndef QT_NO_QWS_MANAGER
    if ( testWFlags(WType_TopLevel) && topData()->qwsManager &&
	 !topData()->decor_allocated_region.isEmpty()) {
	r &= topData()->qwsManager->region();
	if (!r.isEmpty()) {
	    r.translate(-crect.x(),-crect.y());
	    if ( post ) {
		QApplication::postEvent(topData()->qwsManager,
			new QPaintEvent(visibleRect(), TRUE ) );
	    } else {
		QPaintEvent e(r, FALSE);
		setWState( WState_InPaintEvent );
		qt_set_paintevent_clipping( this, r );
		QApplication::sendEvent(topData()->qwsManager, &e );
		qt_clear_paintevent_clipping();
		clearWState( WState_InPaintEvent );
	    }
	}
    }
#endif
}

void QETWidget::updateRegion()
{
    if ( testWFlags(WType_Desktop) )
	return;
    if ( extra && !extra->mask.isNull() ) {
	req_region = extra->mask;
	req_region.translate(crect.x(),crect.y());
	req_region &= crect;
    } else {
	req_region = crect;
    }
    req_region = qt_screen->mapToDevice( req_region, QSize(qt_screen->width(), qt_screen->height()) );
    updateRequestedRegion( mapToGlobal(QPoint(0,0)) );
    QRegion r( req_region );
#ifndef QT_NO_QWS_MANAGER
    QRegion wmr;
    if ( extra && extra->topextra && extra->topextra->qwsManager ) {
	wmr = extra->topextra->qwsManager->region();
	wmr = qt_screen->mapToDevice( wmr, QSize(qt_screen->width(), qt_screen->height()) );
	r += wmr;
    }
#endif
    if ( isVisible() )
	    qwsDisplay()->requestRegion(winId(), r);

    setChildrenAllocatedDirty();
    paintable_region_dirty = TRUE;
    qwsUpdateActivePainters();
}

bool QETWidget::translateRegionModifiedEvent( const QWSRegionModifiedEvent *event )
{
    QWSRegionManager *rgnMan = qt_fbdpy->regionManager();

    if ( alloc_region_index < 0 ) {
	alloc_region_index = rgnMan->find( winId() );

	if ( alloc_region_index < 0 ) {
	    return FALSE;
	}
    }

#ifndef QT_NO_QWS_MANAGER
    QRegion extraExposed;
#endif

    QWSDisplay::grab();
    int revision = *rgnMan->revision( alloc_region_index );
    if ( revision != alloc_region_revision ) {
	alloc_region_revision = revision;
	QRegion newRegion = rgnMan->region( alloc_region_index );
	QWSDisplay::ungrab();
#ifndef QT_NO_QWS_MANAGER
	if ( testWFlags(WType_TopLevel) && topData()->qwsManager ) {
	    if ( event->simpleData.nrectangles && qws_regionRequest ) {
		extraExposed = topData()->decor_allocated_region;
		QSize s( qt_screen->deviceWidth(), qt_screen->deviceHeight() );
		extraExposed = qt_screen->mapFromDevice( extraExposed, s );
		extraExposed &= geometry();
	    }

	    QRegion mr(topData()->qwsManager->region());
	    mr = qt_screen->mapToDevice( mr, QSize(qt_screen->width(), qt_screen->height()) );
	    topData()->decor_allocated_region = newRegion & mr;
	    newRegion -= mr;
	}
#endif
	alloc_region = newRegion;

	// set children's allocated region dirty
	const QObjectList *c = children();
	if ( c ) {
	    QObjectListIt it(*c);
	    QObject* ch;
	    while ((ch=it.current())) {
		++it;
		if ( ch->isWidgetType() ) {
		    ((QWidget *)ch)->alloc_region_dirty = TRUE;
		}
	    }
	}

	paintable_region_dirty = TRUE;
    } else {
	QWSDisplay::ungrab();
    }

    if ( event->simpleData.nrectangles )
    {
	QRegion exposed;
	exposed.setRects( event->rectangles, event->simpleData.nrectangles );
	QSize s( qt_screen->deviceWidth(), qt_screen->deviceHeight() );
	exposed = qt_screen->mapFromDevice( exposed, s );
/*
	for ( int i = 0; i < event->simpleData.nrectangles; i++ )
	    qDebug( "exposed: %d, %d %d x %d",
		event->rectangles[i].x(),
		event->rectangles[i].y(),
		event->rectangles[i].width(),
		event->rectangles[i].height() );
*/
	qwsUpdateActivePainters();
	repaintDecoration( exposed, FALSE );

#ifndef QT_NO_QWS_MANAGER
	exposed |= extraExposed;
#endif

	repaintHierarchy( exposed, FALSE );
    }
    qws_regionRequest = FALSE;
    return TRUE;
}


void  QApplication::setCursorFlashTime( int msecs )
{
    cursor_flash_time = msecs;
}


int QApplication::cursorFlashTime()
{
    return cursor_flash_time;
}

void QApplication::setDoubleClickInterval( int ms )
{
    mouse_double_click_time = ms;
}

int QApplication::doubleClickInterval()
{
    return mouse_double_click_time;
}



/*****************************************************************************
  Session management support
 *****************************************************************************/
#ifndef QT_NO_SESSIONMANAGER

class QSessionManagerData
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QSessionManager::RestartHint restartHint;
};

QSessionManager::QSessionManager( QApplication * app, QString &session )
    : QObject( app, "session manager" )
{
    d = new QSessionManagerData;
    d->sessionId = session;
    d->restartHint = RestartIfRunning;
}

QSessionManager::~QSessionManager()
{
    delete d;
}

QString QSessionManager::sessionId() const
{
    return d->sessionId;
}

bool QSessionManager::allowsInteraction()
{
    return TRUE;
}

bool QSessionManager::allowsErrorInteraction()
{
    return TRUE;
}

void QSessionManager::release()
{
}

void QSessionManager::cancel()
{
}

void QSessionManager::setRestartHint( QSessionManager::RestartHint hint)
{
    d->restartHint = hint;
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
    return d->restartHint;
}

void QSessionManager::setRestartCommand( const QStringList& command)
{
    d->restartCommand = command;
}

QStringList QSessionManager::restartCommand() const
{
    return d->restartCommand;
}

void QSessionManager::setDiscardCommand( const QStringList& command)
{
    d->discardCommand = command;
}

QStringList QSessionManager::discardCommand() const
{
    return d->discardCommand;
}

void QSessionManager::setProperty( const QString&, const QString&)
{
}

void QSessionManager::setProperty( const QString&, const QStringList& )
{
}

bool QSessionManager::isPhase2() const
{
    return FALSE;
}

void QSessionManager::requestPhase2()
{
}
#endif //QT_NO_SESSIONMANAGER
// Need to add some sort of implementation here?

void QApplication::setWheelScrollLines(int)
{
}

int QApplication::wheelScrollLines()
{
    return 0;
}

void QApplication::wakeUpGuiThread()
{
#if defined(QT_THREAD_SUPPORT)
    char c = 0;
    int nbytes;
    if ( ::ioctl(qt_thread_pipe[0], FIONREAD, (char*)&nbytes) >= 0 && nbytes == 0 ) {
	::write(  qt_thread_pipe[1], &c, 1  );
    }
#endif
}

void QApplication::setEffectEnabled( Qt::UIEffect effect, bool enable )
{
    switch (effect) {
    case UI_AnimateMenu:
	animate_menu = enable;
	break;
    case UI_FadeMenu:
	if ( enable )
	    animate_menu = TRUE;
	fade_menu = enable;
	break;
    case UI_AnimateCombo:
	animate_combo = enable;
	break;
    case UI_AnimateTooltip:
	animate_tooltip = enable;
	break;
    case UI_FadeTooltip:
	if ( enable )
	    animate_tooltip = TRUE;
	fade_tooltip = enable;
	break;
    default:
	animate_ui = enable;
	break;
    }
}

bool QApplication::isEffectEnabled( Qt::UIEffect effect )
{
    if ( !animate_ui )
	return FALSE;

    switch( effect ) {
    case UI_AnimateMenu:
	return animate_menu;
    case UI_FadeMenu:
	if ( QColor::numBitPlanes() < 16 )
	    return FALSE;
	return fade_menu;
    case UI_AnimateCombo:
	return animate_combo;
    case UI_AnimateTooltip:
	return animate_tooltip;
    case UI_FadeTooltip:
	if ( QColor::numBitPlanes() < 16 )
	    return FALSE;
	return fade_tooltip;
    default:
	return animate_ui;
    }
}
