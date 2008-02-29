/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwindowsystem_qws.h"
#include "qwsevent_qws.h"
#include "qwscommand_qws_p.h"
#include "qtransportauth_qws_p.h"
#include "qwsutils_qws.h"
#include "qwscursor_qws.h"
#include "qwsdisplay_qws.h"
#include "qmouse_qws.h"
#include "qcopchannel_qws.h"

#include "qapplication.h"
#include "private/qapplication_p.h"
#include "qsocketnotifier.h"
#include "qpolygon.h"
#include "qimage.h"
#include "qcursor.h"
#include <private/qpaintengine_raster_p.h>
#include "qscreen_qws.h"
#include "qwindowdefs.h"
#include "private/qlock_p.h"
#include "qwslock_p.h"
#include "qfile.h"
#include "qtimer.h"
#include "qpen.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qinputcontext.h"
#include "qpainter.h"

#include <qdebug.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifndef QT_NO_QWS_MULTIPROCESS
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#ifndef Q_OS_DARWIN
# include <sys/sem.h>
#endif
#include <sys/param.h>
#include <sys/mount.h>
#endif
#include <signal.h>
#include <fcntl.h>

#if !defined(QT_NO_SOUND) && !defined(Q_OS_DARWIN)
#ifdef QT_USE_OLD_QWS_SOUND
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#else
#include "qsoundqss_qws.h"
#endif
#endif

#include "qkbddriverfactory_qws.h"
#include "qmousedriverfactory_qws.h"

#include <qbuffer.h>

#include <private/qwindowsurface_qws_p.h>

#include "qwindowsystem_p.h"

QWSServer Q_GUI_EXPORT *qwsServer=0;
static QWSServerPrivate *qwsServerPrivate=0;

QWSScreenSaver::~QWSScreenSaver()
{
}

extern QByteArray qws_display_spec;
extern void qt_init_display(); //qapplication_qws.cpp
extern QString qws_qtePipeFilename();

extern void qt_client_enqueue(const QWSEvent *); //qapplication_qws.cpp
extern QList<QWSCommand*> *qt_get_server_queue();

Q_GLOBAL_STATIC_WITH_ARGS(QString, defaultMouse, ("Auto"));
Q_GLOBAL_STATIC_WITH_ARGS(QString, defaultKeyboard, ("TTY"));

static int qws_keyModifiers = 0;

static QWSWindow *keyboardGrabber;
static bool keyboardGrabbing;

static int get_object_id(int count = 1)
{
    static int next=1000;
    int n = next;
    next += count;
    return n;
}
#ifndef QT_NO_QWS_INPUTMETHODS
static QWSInputMethod *current_IM = 0;

static QWSWindow *current_IM_composing_win = 0;
static int current_IM_winId = -1;
static bool force_reject_strokeIM = false;
#endif


//#define QWS_REGION_DEBUG

/*!
    \class QWSScreenSaver
    \ingroup qws

    \brief The QWSScreenSaver class is a base class for screensavers
    in Qtopia Core.

    When running \l {Qtopia Core} applications, it is the server
    application that installs and controls the screensaver. \l {Qtopia
    Core} supports multilevel screen saving, i.e., it is possible to
    specify several different levels of screen responsiveness. For
    example, you can choose to first turn off the light before you
    fully activate the screensaver.

    Note that there exists no default screensaver implementation.

    To create a custom screen driver, derive from this class and
    reimplement the restore() and save() functions. These functions
    are called whenever the screensaver is activated or deactivated,
    respectively. Once an instance of your custom screensaver is
    created, you can use the QWSServer::setScreenSaver() function to
    install it.

    \sa QWSServer, QScreen, {Qtopia Core}
*/

/*!
    \fn QWSScreenSaver::~QWSScreenSaver()

    Reimplement this function to destroy the screensaver.
*/

/*!
    \fn QWSScreenSaver::restore()

    Implement this function to deactivate the screensaver, restoring
    the previously saved screen.

    \sa save(), QWSServer::screenSaverActivate()
*/

/*!
    \fn QWSScreenSaver::save(int level)

    Implement this function to activate the screensaver, saving the
    current screen.

    \l {Qtopia Core} supports multilevel screen saving, i.e., it is
    possible to specify several different levels of screen
    responsiveness. For example, you can choose to first turn off the
    light before you fully activate the screensaver. Use the
    QWSServer::setScreenSaverIntervals() to specify the time intervals
    between the different levels.

    This function should return true if the screensaver successfully
    enters the given \a level; otherwise it should return false.

    \sa restore(), QWSServer::screenSaverActivate()
*/

class QWSWindowPrivate
{
public:
    QWSWindowPrivate();

    QRegion allocatedRegion;
#ifndef QT_NO_QWSEMBEDWIDGET
    QList<QWSWindow*> embedded;
    QWSWindow *embedder;
#endif
};

QWSWindowPrivate::QWSWindowPrivate()
#ifndef QT_NO_QWSEMBEDWIDGET
    : embedder(0)
#endif
{
}

/*!
    \class QWSWindow
    \ingroup qws

    \brief The QWSWindow class encapsulates a top-level window in
    Qtopia Core.

    When running a \l {Qtopia Core} application, it either runs as a
    server or connects to an existing server. As applications add and
    remove widgets, the QWSServer class maintains information about
    each window. Note that you should never construct the QWSWindow
    class yourself; the current top-level windows can be retrieved
    using the QWSServer::clientWindows() function.

    QWSWindow provides functions returning various information about
    the window: its caption(), name(), opacity() and winId() along with
    the client() that owns the window.

    In addition, it is possible to determine whether the window is
    visible using the isVisible() function, if the window is
    completely obscured by another window or by the bounds of the
    screen using the isFullyObscured() function, and whether the
    window has an alpha channel different from 255 using the
    isOpaque() function. Finally, QWSWindow provides the
    requestedRegion() function that returns the region of the display
    the window wants to draw on.

    \sa QWSServer, {Running Qtopia Core Applications}
*/

/*!
    \fn int QWSWindow::winId() const

    Returns the window's ID.

    \sa name(), caption()
*/

/*!
    \fn const QString &QWSWindow::name() const

    Returns the window's name.

    \sa caption(), winId()
*/

/*!
    \fn const QString &QWSWindow::caption() const

    Returns the window's caption.

    \sa name(), winId()
*/

/*!
    \fn QWSClient* QWSWindow::client() const

    Returns a reference to the QWSClient object that owns this window.
*/

/*!
    \fn QRegion QWSWindow::requestedRegion() const

    Returns the region that the window has requested to draw onto,
    including any window decorations.
*/

/*!
    \fn bool QWSWindow::isVisible() const

    Returns true if the window is visible; otherwise returns false.

    \sa isFullyObscured()
*/

/*!
    \fn bool QWSWindow::isOpaque() const

    Returns true if the window is opaque, i.e. if its alpha channel
    equals 255; otherwise returns false.

    \sa opacity()
*/

/*!
    \fn uint QWSWindow::opacity () const

    Returns the window's alpha channel value.

    \sa isOpaque()
*/

/*!
    \fn bool QWSWindow::isPartiallyObscured() const
    \internal

    Returns true if the window is partially obsured by another window
    or by the bounds of the screen; otherwise returns false.
*/

/*!
    \fn bool QWSWindow::isFullyObscured() const

    Returns true if the window is completely obsured by another window
    or by the bounds of the screen; otherwise returns false.

    \sa isVisible()
*/

/*!
    \fn QWSWindowSurface* QWSWindow::windowSurface() const
    \internal
*/

QWSWindow::QWSWindow(int i, QWSClient* client)
        : id(i), modified(false),
          onTop(false), c(client), last_focus_time(0), _opacity(255),
          opaque(true), d(new QWSWindowPrivate)
{
    surface = 0;
}

void QWSWindow::createSurface(const QString &key, const QByteArray &data)
{
    delete surface;
    surface = qt_screen->createSurface(key);
    surface->attach(data);
}

/*!
    \internal
    Raises the window above all other windows except "Stay on top" windows.
*/
void QWSWindow::raise()
{
    qwsServerPrivate->raiseWindow(this);
#ifndef QT_NO_QWSEMBEDWIDGET
    const int n = d->embedded.size();
    for (int i = 0; i < n; ++i)
        d->embedded.at(i)->raise();
#endif
}

/*!
    \internal
    Lowers the window below other windows.
*/
void QWSWindow::lower()
{
    qwsServerPrivate->lowerWindow(this);
#ifndef QT_NO_QWSEMBEDWIDGET
    const int n = d->embedded.size();
    for (int i = 0; i < n; ++i)
        d->embedded.at(i)->lower();
#endif
}

/*!
    \internal
    Shows the window.
*/
void QWSWindow::show()
{
    operation(QWSWindowOperationEvent::Show);
#ifndef QT_NO_QWSEMBEDWIDGET
    const int n = d->embedded.size();
    for (int i = 0; i < n; ++i)
        d->embedded.at(i)->show();
#endif
}

/*!
    \internal
    Hides the window.
*/
void QWSWindow::hide()
{
    operation(QWSWindowOperationEvent::Hide);
#ifndef QT_NO_QWSEMBEDWIDGET
    const int n = d->embedded.size();
    for (int i = 0; i < n; ++i)
        d->embedded.at(i)->hide();
#endif
}

/*!
    \internal
    Make this the active window (i.e. sets the keyboard focus to this
    window).
*/
void QWSWindow::setActiveWindow()
{
    qwsServerPrivate->setFocus(this, true);
#ifndef QT_NO_QWSEMBEDWIDGET
    const int n = d->embedded.size();
    for (int i = 0; i < n; ++i)
        d->embedded.at(i)->setActiveWindow();
#endif
}

void QWSWindow::setName(const QString &n)
{
    rgnName = n;
}

/*!
  \internal
  Sets the window's caption to \a c.
*/
void QWSWindow::setCaption(const QString &c)
{
    rgnCaption = c;
}


static int global_focus_time_counter=100;

void QWSWindow::focus(bool get)
{
    if (get)
        last_focus_time = global_focus_time_counter++;
    if (c) {
        QWSFocusEvent event;
        event.simpleData.window = id;
        event.simpleData.get_focus = get;
        c->sendEvent(&event);
    }

#ifndef QT_NO_QWSEMBEDWIDGET
    const int n = d->embedded.size();
    for (int i = 0; i < n; ++i)
        d->embedded.at(i)->focus(get);
#endif
}

void QWSWindow::operation(QWSWindowOperationEvent::Operation o)
{
    if (!c)
        return;
    QWSWindowOperationEvent event;
    event.simpleData.window = id;
    event.simpleData.op = o;
    c->sendEvent(&event);
}

/*!
    \internal
    Destructor.
*/
QWSWindow::~QWSWindow()
{
#ifndef QT_NO_QWS_INPUTMETHODS
    if (current_IM_composing_win == this)
        current_IM_composing_win = 0;
#endif
#ifndef QT_NO_QWSEMBEDWIDGET
    QWSWindow *embedder = d->embedder;
    if (embedder) {
        embedder->d->embedded.removeAll(this);
        d->embedder = 0;
    }
    while (!d->embedded.isEmpty())
        stopEmbed(d->embedded.first());
#endif
    delete surface;
    delete d;
}

/*!
    \internal

    Returns the region that the window is allowed to draw onto,
    including any window decorations but excluding regions covered by
    other windows.

    \sa requestedRegion()
*/
QRegion QWSWindow::allocatedRegion() const
{
    return d->allocatedRegion;
}

inline void QWSWindow::setAllocatedRegion(const QRegion &region)
{
    d->allocatedRegion = region;
}

#ifndef QT_NO_QWSEMBEDWIDGET
inline void QWSWindow::startEmbed(QWSWindow *w)
{
    d->embedded.append(w);
    w->d->embedder = this;
}

inline void QWSWindow::stopEmbed(QWSWindow *w)
{
    w->d->embedder = 0;
    w->client()->sendEmbedEvent(w->winId(), QWSEmbedEvent::Region, QRegion());
    d->embedded.removeAll(w);
}
#endif // QT_NO_QWSEMBEDWIDGET

/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/

class QWSClientPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWSClient)

public:
    QWSClientPrivate();
    ~QWSClientPrivate();

    void setLockId(int id);
    void unlockCommunication();

private:
#ifndef QT_NO_QWS_MULTIPROCESS
    QWSLock *clientLock;
#endif
    friend class QWSServerPrivate;
};

QWSClientPrivate::QWSClientPrivate()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    clientLock = 0;
#endif
}

QWSClientPrivate::~QWSClientPrivate()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    delete clientLock;
#endif
}

void QWSClientPrivate::setLockId(int id)
{
#ifdef QT_NO_QWS_MULTIPROCESS
    Q_UNUSED(id);
#else
    clientLock = new QWSLock(id);
#endif
}

void QWSClientPrivate::unlockCommunication()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if (clientLock)
        clientLock->unlock(QWSLock::Communication);
#endif
}

/*!
    \class QWSClient
    \ingroup qws

    \brief The QWSClient class encapsulates a client process in Qtopia
    Core.

    When running a \l {Qtopia Core} application, it either runs as a server
    or as a client connected to an existing server. The server is
    responsible for managing top-level window regions. A list of the
    current windows can be retrieved using the
    QWSServer::clientWindows() function, and each window can tell
    which client that owns it through its QWSWindow::client()
    function.

    A QWSClient object has an unique ID that can be retrieved using
    its clientId() function. QWSClient also provides the identity()
    function which typically returns the name of this client's running
    application.

    \sa QWSServer, QCopChannel, {Running Qtopia Core Applications}
*/

/*!
   \internal
*/
//always use frame buffer
QWSClient::QWSClient(QObject* parent, QWS_SOCK_BASE* sock, int id)
    : QObject(*new QWSClientPrivate, parent), command(0), cid(id)
{
#ifdef QT_NO_QWS_MULTIPROCESS
    Q_UNUSED(sock);
    isClosed = false;
#else
    csocket = 0;
    if (!sock) {
        socketDescriptor = -1;
        isClosed = false;
    } else {
        csocket = static_cast<QWSSocket*>(sock); //###
        isClosed = false;

        csocket->flush();
        socketDescriptor = csocket->socketDescriptor();
        connect(csocket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
        connect(csocket, SIGNAL(disconnected()), this, SLOT(closeHandler()));
        connect(csocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorHandler()));
    }
#endif //QT_NO_QWS_MULTIPROCESS
}

/*!
   \internal
*/
QWSClient::~QWSClient()
{
    qDeleteAll(cursors);
    delete command;
}

/*!
   \internal
*/
void QWSClient::setIdentity(const QString& i)
{
    id = i;
}

void QWSClient::closeHandler()
{
    isClosed = true;
    emit connectionClosed();
}

void QWSClient::errorHandler()
{
#ifdef QWS_SOCKET_DEBUG
    qDebug("Client %p error %s", this, csocket ? csocket->errorString().toLatin1().constData() : "(no socket)");
#endif
    isClosed = true;
//####Do we need to clean out the pipes?

    emit connectionClosed();
}

/*!
   \internal
*/
int QWSClient::socket() const
{
    return socketDescriptor;
}

/*!
   \internal
*/
void QWSClient::sendEvent(QWSEvent* event)
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if (csocket) {
        // qDebug() << "QWSClient::sendEvent type " << event->type << " socket state " << csocket->state();
        if ((QAbstractSocket::SocketState)(csocket->state()) == QAbstractSocket::ConnectedState) {
            event->write(csocket);
        }
    }
    else
#endif
    {
        qt_client_enqueue(event);
    }
}

/*!
   \internal
*/
void QWSClient::sendRegionEvent(int winid, QRegion rgn, int type)
{
    QWSRegionEvent event;
    event.simpleData.window = winid;
    event.simpleData.nrectangles = rgn.rects().count();
    event.simpleData.type = type;
    QVector<QRect> rects = rgn.rects();
    event.setData((char *)rects.constData(),
                    rects.count() * sizeof(QRect), false);

//    qDebug() << "Sending Region event to" << winid << "rgn" << rgn << "type" << type;

    sendEvent(&event);
}

extern int qt_servershmid;

/*!
   \internal
*/
void QWSClient::sendConnectedEvent(const char *display_spec)
{
    QWSConnectedEvent event;
    event.simpleData.window = 0;
    event.simpleData.len = strlen(display_spec) + 1;
    event.simpleData.clientId = cid;
    event.simpleData.servershmid = qt_servershmid;
    char * tmp=(char *)display_spec;
    event.setData(tmp, event.simpleData.len);
    sendEvent(&event);
}

/*!
   \internal
*/
void QWSClient::sendMaxWindowRectEvent(const QRect &rect)
{
    QWSMaxWindowRectEvent event;
    event.simpleData.window = 0;
    event.simpleData.rect = rect;
    sendEvent(&event);
}

/*!
   \internal
*/
#ifndef QT_NO_QWS_PROPERTIES
void QWSClient::sendPropertyNotifyEvent(int property, int state)
{
    QWSPropertyNotifyEvent event;
    event.simpleData.window = 0; // not used yet
    event.simpleData.property = property;
    event.simpleData.state = state;
    sendEvent(&event);
}

/*!
   \internal
*/
void QWSClient::sendPropertyReplyEvent(int property, int len, const char *data)
{
    QWSPropertyReplyEvent event;
    event.simpleData.window = 0; // not used yet
    event.simpleData.property = property;
    event.simpleData.len = len;
    event.setData(data, len);
    sendEvent(&event);
}
#endif //QT_NO_QWS_PROPERTIES

/*!
   \internal
*/
void QWSClient::sendSelectionClearEvent(int windowid)
{
    QWSSelectionClearEvent event;
    event.simpleData.window = windowid;
    sendEvent(&event);
}

/*!
   \internal
*/
void QWSClient::sendSelectionRequestEvent(QWSConvertSelectionCommand *cmd, int windowid)
{
    QWSSelectionRequestEvent event;
    event.simpleData.window = windowid;
    event.simpleData.requestor = cmd->simpleData.requestor;
    event.simpleData.property = cmd->simpleData.selection;
    event.simpleData.mimeTypes = cmd->simpleData.mimeTypes;
    sendEvent(&event);
}

#ifndef QT_NO_QWSEMBEDWIDGET
/*!
   \internal
*/
void QWSClient::sendEmbedEvent(int windowid, QWSEmbedEvent::Type type,
                               const QRegion &region)
{
    QWSEmbedEvent event;
    event.setData(windowid, type, region);
    sendEvent(&event);
}
#endif // QT_NO_QWSEMBEDWIDGET

/*!
   \fn void QWSClient::connectionClosed()
   \internal
*/

/*!
    \fn void QWSClient::readyRead();
    \internal
*/

/*!
   \fn int QWSClient::clientId () const

   Returns an integer uniquely identfying this client.
*/

/*!
   \fn QString QWSClient::identity () const

   Returns the name of this client's running application.
*/
/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/

/*!
    \class QWSServer
    \brief The QWSServer class provides server-specific functionality in Qtopia Core.

    \ingroup qws

    When you run a \l {Qtopia Core} application, it either runs as a
    server or connects to an existing server. If it runs as a server,
    the QWSServer class provides additional functionality to handle
    the clients as well as the mouse, keyboard, display and input
    methods.

    Note that this class is instantiated by QApplication for \l
    {Qtopia Core} server processes; you should never construct this
    class yourself. Access to the QWSServer instance can be obtained
    using the global qwsServer pointer.

    \tableofcontents

    \section1 Client Administration

    In \l {Qtopia Core}, the top-level windows are encasulated as
    QWSWindow objects. The collection of windows changes as
    applications add and remove widgets, and each window can tell
    which client that owns it through its QWSWindow::client()
    function. Use the clientWindows() function to retrieve a list of
    the current top-level windows. Given a particular position on the
    display, the window containing it can be retrieved using the
    windowAt() function.

    QWSServer also provides the windowEvent() signal which is emitted
    whenever something happens to a top level window; the WindowEvent
    enum describes the various types of events that the signal
    recognizes. In addition, the server class provides the
    markedText() signal which is emitted whenever some text has been
    selected in any of the windows, passing the selection as
    parameter.

    See the QWSWindow and QWSClient class documentation for more
    details.

    The QCopChannel class and the QCOP communication protocol enable
    transfer of messages between clients. QWSServer provides the
    newChannel() and removedChannel() signals that is emitted whenever
    a new QCopChannel object is created or destroyed. See the
    QCopChannel class documentation for details on client
    communication.

    \section1 Mouse and Keyboard Handling

    To open the mouse devices specified by the QWS_MOUSE_PROTO
    environment variable, use the openMouse() function. Alternatively,
    the static setDefaultMouse() function provides means of specifying
    the default mouse driver to use if the QWS_MOUSE_PROTO variable is
    not defined. Note that the default is otherwise platform
    dependent. The primary mouse handler can be retrieved using the
    static mouseHandler() function. Use the closeMouse() function to
    delete the mouse handlers.

    Likewise, open the keyboard devices specified by the QWS_KEYBOARD
    environment variable by using the openKeyboard()
    function. Alternatively, the static setDefaultKeyboard() function
    provides means of specifying the default keyboard driver to use if
    the QWS_KEYBOARD variable is not defined. Note again tha the
    default is otherwise platform dependent.  The primary keyboard
    handler can be retrieved using the static keyboardHandler()
    function. Use the closeKeyboard() function to delete the keyboard
    handlers.

    In addition, the QWSServer class can control the flow of mouse
    input using the suspendMouse() and resumeMouse() functions (see
    QWSMouseHandler for more details), and handle key events from both
    physical and virtual keyboards using the processKeyEvent() and
    sendKeyEvent() functions, respectively. Use the
    addKeyboardFilter() function to filter the key events from
    physical keyboard drivers, the most recently added filter can be
    removed and deleted using the removeKeyboardFilter() function. See
    \l {Qtopia Core Character Input} and QWSKeyboardHandler for more
    information.

    Finally, the keyMap() function returns the keyboard mapping table
    used to convert keyboard scancodes to Qt keycodes and Unicode
    values.

    \section1 Display Handling

    To set the brush used as the background in the absence of
    obscuring windows, QWSServer provides the static setBackground()
    function. The backgroundBrush() function returns the currently set
    brush.

    Use the refresh() function to refresh the entire display, or
    alternatively a specified region of it. The enablePainting()
    function can be used to control the privileges for painting on the
    display. QWSServer also provide the setMaxWindowRect() function
    restricting the area of the screen which \l {Qtopia Core}
    applications will consider to be the maximum area to use for
    windows.

    The setScreenSaver() function provides the option of installing a
    custom screensaver derived from the QWSScreenSaver class. Once
    installed, the screensaver can be activated using the
    screenSaverActivate() function, and the screenSaverActive()
    function returns its current status. The screensaver's timeout
    intervals can be specified using the setScreenSaverInterval() and
    setScreenSaverIntervals() functions.

    Use the isCursorVisible() function to determine if the cursor is
    visible on the display, use the setCursorVisible() function to
    alter its visibility.

    \section1 Input Method Handling

    Custom input methods derived from the QWSInputMethod class, can be
    installed using the setCurrentInputMethod(). Use the sendIMEvent()
    and sendIMQuery() functions to send input method events and
    queries. Finally, QWSServer provides the IMMouse enum describing
    the various mouse events recognized by the
    QWSInputMethod::mouseHandler() function. The latter function
    allows subclasses of QWSInputMethod to handle mouse events within
    the preedit text.

    \sa QWSClient, {Running Qtopia Core Applications}
*/

/*!
    \enum QWSServer::IMState
    \obsolete

    This enum describes the various states of an input method.

    \value IMCompose Composing.
    \value IMStart Equivalent to IMCompose.
    \value IMEnd Finished composing.

    \sa QWSInputMethod::sendIMEvent()
*/

/*!
    \enum QWSServer::IMMouse

    This enum describes the various types of mouse events recognized
    by the QWSInputMethod::mouseHandler() function which allows
    subclasses of QWSInputMethod to handle mouse events within the
    preedit text.

    \value MousePress An event generated by pressing a mouse button.
    \value MouseRelease An event generated by relasing a mouse button.
    \value MouseMove An event generated by moving the mouse cursor.
    \value MouseOutside This value is only reserved, i.e. it is not used in
                                    current implementations.

    \sa QWSInputMethod::mouseHandler()
*/

/*!
    \enum QWSServer::ServerFlags
    \internal

    This enum is used to pass various options to the window system
    server.

    \value DisableKeyboard Ignore all keyboard input.
    \value DisableMouse Ignore all mouse input.
*/

/*!
    \enum QWSServer::WindowEvent

    This enum specifies the various events that can occur in a
    top-level window.

    \value Create A new window has been created (by the QWidget constructor).
    \value Destroy The window has been closed and deleted (by the QWidget destructor).
    \value Hide The window has been hidden using the QWidget::hide() function.
    \value Show The window has been shown using the QWidget::show() function or similar.
    \value Raise The window has been raised to the top of the desktop.
    \value Lower The window has been lowered.
    \value Geometry The window has changed size or position.
    \value Active The window has become the active window (i.e. it has keyboard focus).
    \value Name The window has been named.

    \sa windowEvent()
*/

/*!
    \fn void QWSServer::markedText(const QString &selection)

    This signal is emitted whenever some text is selected in any of
    the running applications, passing the selected text in the \a
    selection parameter.
*/

/*!
    \fn const QList<QWSWindow*> &QWSServer::clientWindows()

    Returns the list of current top-level windows.

    Note that the collection of top-level windows changes as
    applications add and remove widgets so it should not be stored for
    future use. The windows are sorted in stacking order from top-most
    to bottom-most.

    \sa windowAt(),  QWSClient
*/

/*!
    \fn void QWSServer::newChannel(const QString& channel)

    This signal is emitted whenever a new QCopChannel object is
    created, passing the channel's name in the \a channel parameter.

    \sa removedChannel()
*/

/*!
    \fn void QWSServer::removedChannel(const QString& channel)

    This signal is emitted immediately after the QCopChannel object
    specified by \a channel, is destroyed.

    Note that a channel is not destroyed until all its listeners have
    been unregistered.

    \sa newChannel()
*/

/*!
    \fn QWSServer::QWSServer(int flags, QObject *parent)
    \internal

    Construct a QWSServer object with the given \a parent.  The \a
    flags are used for keyboard and mouse settings.

    \warning This class is instantiated by QApplication for
    \l {Qtopia Core} server processes. You should never construct this
    class yourself.

    \sa {Running Applications}
*/

/*!
    \fn static QWSServer* QWSServer::instance()
    \since 4.2

    Returns a pointer to the application's QWSServer instance. The pointer
    will be 0 if the application is not the window server, e.g.
    QApplication::type() != QApplication::GuiServer.
*/

struct QWSCommandStruct
{
    QWSCommandStruct(QWSCommand *c, QWSClient *cl) :command(c),client(cl){}
    ~QWSCommandStruct() { delete command; }

    QWSCommand *command;
    QWSClient *client;

};

QWSServer::QWSServer(int flags, QObject *parent) :
    QObject(*new QWSServerPrivate, parent)
{
    Q_D(QWSServer);
    d->initServer(flags);
}

#ifdef QT3_SUPPORT
/*!
    Use the two-argument overload and call setObjectName() instead.
*/
QWSServer::QWSServer(int flags, QObject *parent, const char *name) :
    QObject(*new QWSServerPrivate, parent)
{
    Q_D(QWSServer);
    setObjectName(QString::fromAscii(name));
    d->initServer(flags);
}
#endif


#ifndef QT_NO_QWS_MULTIPROCESS
static void ignoreSignal(int) {} // Used to eat SIGPIPE signals below
#endif

void QWSServerPrivate::initServer(int flags)
{
    Q_Q(QWSServer);
    Q_ASSERT(!qwsServer);
    qwsServer = q;
    qwsServerPrivate = this;
    disablePainting = false;
#ifndef QT_NO_QWS_MULTIPROCESS
    ssocket = new QWSServerSocket(qws_qtePipeFilename(), q);
    QObject::connect(ssocket, SIGNAL(newConnection()), q, SLOT(_q_newConnection()));

    if ( !ssocket->isListening()) {
        perror("QWSServerPrivate::initServer: server socket not listening");
        qFatal("Failed to bind to %s", qws_qtePipeFilename().toLatin1().constData());
    }

    struct linger tmp;
    tmp.l_onoff=1;
    tmp.l_linger=0;
    setsockopt(ssocket->socketDescriptor(),SOL_SOCKET,SO_LINGER,(char *)&tmp,sizeof(tmp));


    signal(SIGPIPE, ignoreSignal); //we get it when we read
#endif
    focusw = 0;
    mouseGrabber = 0;
    mouseGrabbing = false;
    keyboardGrabber = 0;
    keyboardGrabbing = false;
#ifndef QT_NO_QWS_CURSOR
    haveviscurs = false;
    cursor = 0;
    nextCursor = 0;
#endif

#ifndef QT_NO_QWS_MULTIPROCESS

    if (!geteuid()) {
#if !defined(Q_OS_FREEBSD) && !defined(Q_OS_SOLARIS) && !defined(Q_OS_DARWIN)
        if(mount(0,"/var/shm", "shm", 0, 0)) {
            /* This just confuses people with 2.2 kernels
            if (errno != EBUSY)
                qDebug("Failed mounting shm fs on /var/shm: %s",strerror(errno));
            */
        }
#endif
    }
#endif

    // no selection yet
    selectionOwner.windowid = -1;
    selectionOwner.time.set(-1, -1, -1, -1);

    openDisplay();

    screensavertimer = new QTimer(q);
    screensavertimer->setSingleShot(true);
    QObject::connect(screensavertimer, SIGNAL(timeout()), q, SLOT(_q_screenSaverTimeout()));
    _q_screenSaverWake();

    clientMap[-1] = new QWSClient(q, 0, 0);

    if (!bgBrush)
        bgBrush = new QBrush(QColor(0x20, 0xb0, 0x50));

    initializeCursor();

    // input devices
    if (!(flags&QWSServer::DisableMouse)) {
        q->openMouse();
    }
#ifndef QT_NO_QWS_KEYBOARD
    if (!(flags&QWSServer::DisableKeyboard)) {
        q->openKeyboard();
    }
#endif

#if !defined(QT_NO_SOUND) && !defined(QT_EXTERNAL_SOUND_SERVER) && !defined(Q_OS_DARWIN)
    soundserver = new QWSSoundServer(q);
#endif
}

/*!
    \internal
    Destructs this server.
*/
QWSServer::~QWSServer()
{
    closeMouse();
#ifndef QT_NO_QWS_KEYBOARD
    closeKeyboard();
#endif
}


const QList<QWSWindow*> &QWSServer::clientWindows()
{
    Q_D(QWSServer);
    return d->windows;
}

/*!
  \internal
*/
void QWSServerPrivate::releaseMouse(QWSWindow* w)
{
    if (w && mouseGrabber == w) {
        mouseGrabber = 0;
        mouseGrabbing = false;
#ifndef QT_NO_QWS_CURSOR
        if (nextCursor) {
            // Not grabbing -> set the correct cursor
            setCursor(nextCursor);
            nextCursor = 0;
        }
#endif
    }
}

/*!
  \internal
*/
void QWSServerPrivate::releaseKeyboard(QWSWindow* w)
{
    if (keyboardGrabber == w) {
        keyboardGrabber = 0;
        keyboardGrabbing = false;
    }
}

void QWSServerPrivate::handleWindowClose(QWSWindow *w)
{
    w->shuttingDown();
    if (focusw == w)
        setFocus(w,false);
    if (mouseGrabber == w)
        releaseMouse(w);
    if (keyboardGrabber == w)
        releaseKeyboard(w);
}


#ifndef QT_NO_QWS_MULTIPROCESS
/*!
  \internal
*/
void QWSServerPrivate::_q_newConnection()
{
    Q_Q(QWSServer);
    while (QWS_SOCK_BASE *sock = ssocket->nextPendingConnection()) {
        int socket = sock->socketDescriptor();

        QWSClient *client = new QWSClient(q,sock, get_object_id());
        clientMap[socket] = client;

#ifndef QT_NO_SXE
#ifdef QTRANSPORTAUTH_DEBUG
        qDebug( "Transport auth connected: unix stream socket %d", socket );
#endif
        // get a handle to the per-process authentication service
        QTransportAuth *a = QTransportAuth::getInstance();

        // assert that this transport is trusted
        QTransportAuth::Data *d = a->connectTransport(
                QTransportAuth::UnixStreamSock |
                QTransportAuth::Trusted, socket );

        QAuthDevice *ad = a->recvBuf( d, sock );
        ad->setClient(client);

        QObject::connect(ad, SIGNAL(readyRead()),
                q, SLOT(_q_doClient()));

        QObject::connect(client, SIGNAL(connectionClosed()),
                q, SLOT(_q_clientClosed()));
#else
        QObject::connect(client, SIGNAL(readyRead()),
                         q, SLOT(_q_doClient()));
        QObject::connect(client, SIGNAL(connectionClosed()),
                         q, SLOT(_q_clientClosed()));
#endif // QT_NO_SXE

        client->sendConnectedEvent(qws_display_spec.constData());

        if (clientMap.contains(socket)) {
            QList<QScreen*> screens = qt_screen->subScreens();
            if (screens.isEmpty())
                screens.append(qt_screen);
            for (int i = 0; i < screens.size(); ++i) {
                const QApplicationPrivate *ap = QApplicationPrivate::instance();
                const QRect rect = ap->maxWindowRect(screens.at(i));
                if (!rect.isEmpty())
                    client->sendMaxWindowRectEvent(rect);
            }
        }

        // pre-provide some object id's
        QWSCreateCommand cmd(30);
        invokeCreate(&cmd, client);
    }
}
/*!
  \internal
*/
void QWSServerPrivate::_q_clientClosed()
{
    Q_Q(QWSServer);
    QWSClient* cl = (QWSClient*)q->sender();

    // Remove any queued commands for this client
    int i = 0;
    while (i < commandQueue.size()) {
        QWSCommandStruct *cs = commandQueue.at(i);
        if (cs->client == cl) {
            commandQueue.removeAt(i);
            delete cs;
        } else {
            ++i;
        }
    }

#ifndef QT_NO_COP
    // Enfore unsubscription from all channels.
    QCopChannel::detach(cl);
#endif

    // Shut down all windows for this client
    for (int i = 0; i < windows.size(); ++i) {
        QWSWindow* w = windows.at(i);
        if (w->forClient(cl))
            w->shuttingDown();
    }

    // Delete all windows for this client
    QRegion exposed;
    i = 0;
    while (i < windows.size()) {
        QWSWindow* w = windows.at(i);
        if (w->forClient(cl)) {
            w->c = 0; //so we don't send events to it anymore
            releaseMouse(w);
            releaseKeyboard(w);
            exposed += w->allocatedRegion();
//                rgnMan->remove(w->allocationIndex());
            if (focusw == w)
                setFocus(focusw,0);
            if (mouseGrabber == w)
                releaseMouse(w);
            windows.takeAt(i);
            if (i < nReserved)
                --nReserved;
#ifndef QT_NO_QWS_PROPERTIES
            propertyManager.removeProperties(w->winId());
#endif
            emit q->windowEvent(w, QWSServer::Destroy);
            deletedWindows.append(w);
        } else {
            ++i;
        }
    }
    if (deletedWindows.count())
        QTimer::singleShot(0, q, SLOT(_q_deleteWindowsLater()));

    //qDebug("removing client %d with socket %d", cl->clientId(), cl->socket());
    clientMap.remove(cl->socket());
    if (cl == cursorClient)
        cursorClient = 0;
    if (qt_screen->clearCacheFunc)
        (qt_screen->clearCacheFunc)(qt_screen, cl->clientId());  // remove any remaining cache entries.
    cl->deleteLater();

    update_regions();
    exposeRegion(exposed);
}

void QWSServerPrivate::_q_deleteWindowsLater()
{
    qDeleteAll(deletedWindows);
    deletedWindows.clear();
}

#endif //QT_NO_QWS_MULTIPROCESS

/*!
   \internal
*/
QWSCommand* QWSClient::readMoreCommand()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    QIODevice *socket = 0;
#endif
#ifndef QT_NO_SXE
    if (socketDescriptor != -1)  // not server socket
        socket = QTransportAuth::getInstance()->passThroughByClient( this );
#if QTRANSPORTAUTH_DEBUG
    if (socket) {
        char displaybuf[1024];
        qint64 bytes = socket->bytesAvailable();
        if ( bytes > 511 ) bytes = 511;
        hexstring( displaybuf, ((unsigned char *)(reinterpret_cast<QAuthDevice*>(socket)->buffer().constData())), bytes );
        qDebug( "readMoreCommand: %lli bytes - %s", socket->bytesAvailable(), displaybuf );
    }
#endif
#endif // QT_NO_SXE

#ifndef QT_NO_QWS_MULTIPROCESS
    if (!socket)
        socket = csocket;   // server socket
    if (socket) {
        // read next command
        if (!command) {
            int command_type = qws_read_uint(socket);

            if (command_type >= 0)
                command = QWSCommand::factory(command_type);
        }
        if (command) {
            if (command->read(socket)) {
                // Finished reading a whole command.
                QWSCommand* result = command;
                command = 0;
                return result;
            }
        }

        // Not finished reading a whole command.
        return 0;
    } else
#endif // QT_NO_QWS_MULTIPROCESS
    {
        QList<QWSCommand*> *serverQueue = qt_get_server_queue();
        return serverQueue->isEmpty() ? 0 : serverQueue->takeFirst();
    }
}


/*!
  \internal
*/
void QWSServer::processEventQueue()
{
    if (qwsServerPrivate)
        qwsServerPrivate->doClient(qwsServerPrivate->clientMap.value(-1));
}


#ifndef QT_NO_QWS_MULTIPROCESS
void QWSServerPrivate::_q_doClient()
{
    Q_Q(QWSServer);
    static bool active = false;
    if (active) {
        qDebug("QWSServer::_q_doClient() reentrant call, ignoring");
        return;
    }
    active = true;

    QWSClient* client;
#ifndef QT_NO_SXE
    QAuthDevice *ad = qobject_cast<QAuthDevice*>(q->sender());
    if (ad)
        client = (QWSClient*)ad->client();
    else
#endif
        client = (QWSClient*)q->sender();
    doClient(client);
    active = false;
}
#endif // QT_NO_QWS_MULTIPROCESS

void QWSServerPrivate::doClient(QWSClient *client)
{
    QWSCommand* command=client->readMoreCommand();

    while (command) {
        QWSCommandStruct *cs = new QWSCommandStruct(command, client);
        commandQueue.append(cs);
        // Try for some more...
        command=client->readMoreCommand();
    }

    while (!commandQueue.isEmpty()) {
        QWSCommandStruct *cs = commandQueue.takeAt(0);
        switch (cs->command->type) {
        case QWSCommand::Identify:
            invokeIdentify((QWSIdentifyCommand*)cs->command, cs->client);
            break;
        case QWSCommand::Create:
            invokeCreate((QWSCreateCommand*)cs->command, cs->client);
            break;
        case QWSCommand::RegionName:
            invokeRegionName((QWSRegionNameCommand*)cs->command, cs->client);
            break;
        case QWSCommand::Region:
            invokeRegion((QWSRegionCommand*)cs->command, cs->client);
            cs->client->d_func()->unlockCommunication();
            break;
        case QWSCommand::RegionMove:
            invokeRegionMove((QWSRegionMoveCommand*)cs->command, cs->client);
            cs->client->d_func()->unlockCommunication();
            break;
        case QWSCommand::RegionDestroy:
            invokeRegionDestroy((QWSRegionDestroyCommand*)cs->command, cs->client);
            break;
#ifndef QT_NO_QWS_PROPERTIES
        case QWSCommand::AddProperty:
            invokeAddProperty((QWSAddPropertyCommand*)cs->command);
            break;
        case QWSCommand::SetProperty:
            invokeSetProperty((QWSSetPropertyCommand*)cs->command);
            break;
        case QWSCommand::RemoveProperty:
            invokeRemoveProperty((QWSRemovePropertyCommand*)cs->command);
            break;
        case QWSCommand::GetProperty:
            invokeGetProperty((QWSGetPropertyCommand*)cs->command, cs->client);
            break;
#endif
        case QWSCommand::SetSelectionOwner:
            invokeSetSelectionOwner((QWSSetSelectionOwnerCommand*)cs->command);
            break;
        case QWSCommand::RequestFocus:
            invokeSetFocus((QWSRequestFocusCommand*)cs->command, cs->client);
            break;
        case QWSCommand::ChangeAltitude:
            invokeSetAltitude((QWSChangeAltitudeCommand*)cs->command,
                               cs->client);
            cs->client->d_func()->unlockCommunication();
            break;
        case QWSCommand::SetOpacity:
            invokeSetOpacity((QWSSetOpacityCommand*)cs->command,
                               cs->client);
            break;

#ifndef QT_NO_QWS_CURSOR
        case QWSCommand::DefineCursor:
            invokeDefineCursor((QWSDefineCursorCommand*)cs->command, cs->client);
            break;
        case QWSCommand::SelectCursor:
            invokeSelectCursor((QWSSelectCursorCommand*)cs->command, cs->client);
            break;
        case QWSCommand::PositionCursor:
            invokePositionCursor((QWSPositionCursorCommand*)cs->command, cs->client);
            break;
#endif
        case QWSCommand::GrabMouse:
            invokeGrabMouse((QWSGrabMouseCommand*)cs->command, cs->client);
            break;
        case QWSCommand::GrabKeyboard:
            invokeGrabKeyboard((QWSGrabKeyboardCommand*)cs->command, cs->client);
            break;
#if !defined(QT_NO_SOUND) && !defined(Q_OS_DARWIN)
        case QWSCommand::PlaySound:
            invokePlaySound((QWSPlaySoundCommand*)cs->command, cs->client);
            break;
#endif
#ifndef QT_NO_COP
        case QWSCommand::QCopRegisterChannel:
            invokeRegisterChannel((QWSQCopRegisterChannelCommand*)cs->command,
                                   cs->client);
            break;
        case QWSCommand::QCopSend:
            invokeQCopSend((QWSQCopSendCommand*)cs->command, cs->client);
            break;
#endif
#ifndef QT_NO_QWS_INPUTMETHODS
        case QWSCommand::IMUpdate:
            invokeIMUpdate((QWSIMUpdateCommand*)cs->command, cs->client);
            break;
        case QWSCommand::IMResponse:
            invokeIMResponse((QWSIMResponseCommand*)cs->command, cs->client);
            break;
        case QWSCommand::IMMouse:
            {
                if (current_IM) {
                    QWSIMMouseCommand *cmd = (QWSIMMouseCommand *) cs->command;
                    current_IM->mouseHandler(cmd->simpleData.index,
                                              cmd->simpleData.state);
                }
            }
            break;
#endif
        case QWSCommand::RepaintRegion:
            invokeRepaintRegion((QWSRepaintRegionCommand*)cs->command,
                                cs->client);
            cs->client->d_func()->unlockCommunication();
            break;
#ifndef QT_NO_QWSEMBEDWIDGET
        case QWSCommand::Embed:
            invokeEmbed(static_cast<QWSEmbedCommand*>(cs->command),
                        cs->client);
            break;
#endif
        }
        delete cs;
    }
}


void QWSServerPrivate::showCursor()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->show();
#endif
}

void QWSServerPrivate::hideCursor()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->hide();
#endif
}

/*!
    \fn void QWSServer::enablePainting(bool enable)

    If \a enable is true, painting on the display is enabled;
    otherwise painting is disabled.

    \sa QDirectPainter, QScreen
*/
void QWSServer::enablePainting(bool e)
{
    Q_D(QWSServer);
    // ### don't like this
    if (e)
    {
        d->disablePainting = false;
        d->setWindowRegion(0, QRegion());
        d->showCursor();
    }
    else
    {
        d->disablePainting = true;
        d->hideCursor();
        d->setWindowRegion(0, QRegion(0,0,d->swidth,d->sheight));
    }
}

/*!
    Refreshes the entire display.
*/
void QWSServer::refresh()
{
    Q_D(QWSServer);
    d->exposeRegion(QRegion(0, 0, d->swidth, d->sheight));
//### send repaint to non-buffered windows
}

/*!
    \fn void QWSServer::refresh(QRegion & region)
    \overload

    Refreshes the given \a region of the display.
*/
void QWSServer::refresh(QRegion & r)
{
    Q_D(QWSServer);
    d->exposeRegion(r);
//### send repaint to non-buffered windows
}

/*!
    \fn void QWSServer::setMaxWindowRect(const QRect& rectangle)

    Sets the area of the screen which \l {Qtopia Core} applications
    will consider to be the maximum area to use for windows, to the
    area specified by the given \a rectangle.

    \sa QWidget::showMaximized()
*/
void QWSServer::setMaxWindowRect(const QRect &rect)
{
    QList<QScreen*> subScreens = qt_screen->subScreens();
    if (subScreens.isEmpty() && qt_screen != 0)
        subScreens.append(qt_screen);

    for (int i = 0; i < subScreens.size(); ++i) {
        const QScreen *screen = subScreens.at(i);
        const QRect screenRect = (screen->region() & rect).boundingRect();
        if (screenRect.isEmpty())
            continue;

        const QSize screenSize(screen->width(), screen->height());
        const QRect r = screen->mapToDevice(screenRect, screenSize);
        QApplicationPrivate *ap = QApplicationPrivate::instance();
        if (ap->maxWindowRect(screen) != r) {
            ap->setMaxWindowRect(screen, r);
            qwsServerPrivate->sendMaxWindowRectEvents(r);
        }
    }
}

/*!
  \internal
*/
void QWSServerPrivate::sendMaxWindowRectEvents(const QRect &rect)
{
    QMap<int,QWSClient*>::const_iterator it = clientMap.constBegin();
    for (; it != clientMap.constEnd(); ++it)
        (*it)->sendMaxWindowRectEvent(rect);
}

/*!
    \fn void QWSServer::setDefaultMouse(const char *mouseDriver)

    Sets the mouse driver that is used if the QWS_MOUSE_PROTO
    environment variable is not defined, to be the given \a
    mouseDriver. The default is platform-dependent.

    \sa {Qtopia Core Pointer Handling}
*/
void QWSServer::setDefaultMouse(const char *m)
{
    *defaultMouse() = QString::fromAscii(m);
}

/*!
    \fn void QWSServer::setDefaultKeyboard(const char *keyboardDriver)

    Sets the keyboard driver that is used if the QWS_KEYBOARD
    environment variable is not defined, to be the given \a
    keyboardDriver. The default is platform-dependent.

    \sa {Qtopia Core Character Input}
*/
void QWSServer::setDefaultKeyboard(const char *k)
{
    *defaultKeyboard() = QString::fromAscii(k);
}

#ifndef QT_NO_QWS_CURSOR
static bool prevWin;
#endif


extern int *qt_last_x,*qt_last_y;


/*!
  \internal

  Send a mouse event. \a pos is the screen position where the mouse
  event occurred and \a state is a mask indicating which buttons are
  pressed.

  \a pos is in device coordinates
*/
void QWSServer::sendMouseEvent(const QPoint& pos, int state, int wheel)
{
    //const int btnMask = Qt::LeftButton | Qt::RightButton | Qt::MidButton;
    qwsServerPrivate->showCursor();

    if (state)
        qwsServerPrivate->_q_screenSaverWake();


    QPoint tpos;
    // transformations
    if (qt_screen->isTransformed()) {
	QSize s = QSize(qt_screen->deviceWidth(), qt_screen->deviceHeight());
	tpos = qt_screen->mapFromDevice(pos, s);
    } else {
	tpos = pos;
    }

    if (qt_last_x) {
         *qt_last_x = tpos.x();
         *qt_last_y = tpos.y();
    }
    QWSServer::mousePosition = tpos;
    qwsServerPrivate->mouseState = state;

#ifndef QT_NO_QWS_INPUTMETHODS
    const int btnMask = Qt::LeftButton | Qt::RightButton | Qt::MidButton;
    int stroke_count; // number of strokes to keep shown.
    if (force_reject_strokeIM || qwsServerPrivate->mouseGrabber
	    || !current_IM) {
	stroke_count = 0;
    } else {
	stroke_count = current_IM->filter(tpos, state, wheel);
    }

    if (stroke_count == 0) {
	if (state&btnMask)
	    force_reject_strokeIM = true;
        QWSServerPrivate::sendMouseEventUnfiltered(tpos, state, wheel);
    }
    // stop force reject after stroke ends.
    if (state&btnMask && force_reject_strokeIM)
	force_reject_strokeIM = false;
    // on end of stroke, force_rejct
    // and once a stroke is rejected, do not try again till pen is lifted
#else
    QWSServerPrivate::sendMouseEventUnfiltered(tpos, state, wheel);
#endif // end QT_NO_QWS_FSIM
}

void QWSServerPrivate::sendMouseEventUnfiltered(const QPoint &pos, int state, int wheel)
{
    const int btnMask = Qt::LeftButton | Qt::RightButton | Qt::MidButton;
    QWSMouseEvent event;

    //If grabbing window disappears, grab is still active until
    //after mouse release.
    QWSWindow *win = qwsServerPrivate->mouseGrabber ? qwsServerPrivate->mouseGrabber : qwsServer->windowAt(pos);
    event.simpleData.window = win ? win->id : 0;

#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->move(pos.x(),pos.y());

    // Arrow cursor over desktop
    // prevWin remembers if the last event was over a window
    if (!win && prevWin) {
        if (!qwsServerPrivate->mouseGrabber)
            qwsServerPrivate->setCursor(QWSCursor::systemCursor(Qt::ArrowCursor));
        else
            qwsServerPrivate->nextCursor = QWSCursor::systemCursor(Qt::ArrowCursor);
        prevWin = false;
    }
    // reset prevWin
    if (win && !prevWin)
        prevWin = true;
#endif

    if ((state&btnMask) && !qwsServerPrivate->mouseGrabbing) {
        qwsServerPrivate->mouseGrabber = win;
    }

    event.simpleData.x_root=pos.x();
    event.simpleData.y_root=pos.y();
    event.simpleData.state=state | qws_keyModifiers;
    event.simpleData.delta = wheel;
    event.simpleData.time=qwsServerPrivate->timer.elapsed();

    QWSClient *serverClient = qwsServerPrivate->clientMap.value(-1);
    QWSClient *winClient = win ? win->client() : 0;

#ifndef QT_NO_QWS_INPUTMETHODS
    //tell the input method if we click on a different window that is not IM transparent

    static int oldstate = 0;
    bool isPress = state > oldstate;
    oldstate = state;
    if (isPress && current_IM && current_IM_winId != -1) {
        QWSWindow *kbw = keyboardGrabber ? keyboardGrabber :
                         qwsServerPrivate->focusw;

        QWidget *target = winClient == serverClient ?
                          QApplication::widgetAt(pos) : 0;
        if (kbw != win && (!target || !(target->testAttribute(Qt::WA_InputMethodTransparent))))
            current_IM->mouseHandler(-1, QWSServer::MouseOutside);
    }
#endif

    if (serverClient)
       serverClient->sendEvent(&event);
    if (winClient && winClient != serverClient)
       winClient->sendEvent(&event);

    // Make sure that if we leave a window, that window gets one last mouse
    // event so that it knows the mouse has left.
    QWSClient *oldClient = qwsServer->d_func()->cursorClient;
    if (oldClient && oldClient != winClient && oldClient != serverClient)
        oldClient->sendEvent(&event);

    qwsServer->d_func()->cursorClient = winClient;

    if (!(state&btnMask) && !qwsServerPrivate->mouseGrabbing)
        qwsServerPrivate->releaseMouse(qwsServerPrivate->mouseGrabber);
}

/*!
    Returns the primary mouse handler.

    \sa closeMouse(), openMouse()
*/
QWSMouseHandler *QWSServer::mouseHandler()
{
    if (qwsServerPrivate->mousehandlers.empty())
        return 0;
    return qwsServerPrivate->mousehandlers.first();
}

// called by QWSMouseHandler constructor, not user code.
/*!
    \fn void QWSServer::setMouseHandler(QWSMouseHandler* handler)

    Sets the mouse driver to be the given \a handler.

    \l {Qtopia Core} provides several ready-made mouse drivers, and
    custom drivers are typically added using Qt's plugin
    mechanism. See the {Qtopia Core Pointer Handling} documentation
    for details.

    \sa mouseHandler()
*/
void QWSServer::setMouseHandler(QWSMouseHandler* mh)
{
    if (!mh)
        return;
    qwsServerPrivate->mousehandlers.removeAll(mh);
    qwsServerPrivate->mousehandlers.prepend(mh);
}

/*!
  \internal

  Caller owns data in list, and must delete contents
*/
QList<QWSInternalWindowInfo*> * QWSServer::windowList()
{
    QList<QWSInternalWindowInfo*> * ret=new QList<QWSInternalWindowInfo*>;
    for (int i=0; i < qwsServerPrivate->windows.size(); ++i) {
        QWSWindow *window = qwsServerPrivate->windows.at(i);
        QWSInternalWindowInfo * qwi=new QWSInternalWindowInfo();
        qwi->winid=window->winId();
        qwi->clientid=window->client()->clientId();
#ifndef QT_NO_QWS_PROPERTIES
        const char * name;
        int len;
        qwsServerPrivate->propertyManager.getProperty(qwi->winid,
                                               QT_QWS_PROPERTY_WINDOWNAME,
                                               name,len);
        if(name) {
            char * buf=(char *)malloc(len+2);
            strncpy(buf,name,len);
            buf[len]=0;
            qwi->name=buf;
            free(buf);
        } else {
            qwi->name = QLatin1String("unknown");
        }
#else
        qwi->name = QLatin1String("unknown");
#endif
        ret->append(qwi);
    }

    return ret;
}

#ifndef QT_NO_COP
/*!
  \internal
*/
void QWSServerPrivate::sendQCopEvent(QWSClient *c, const QString &ch,
                               const QString &msg, const QByteArray &data,
                               bool response)
{
    Q_ASSERT(c);

    QWSQCopMessageEvent event;
    event.simpleData.is_response = response;
    event.simpleData.lchannel = ch.length();
    event.simpleData.lmessage = msg.length();
    event.simpleData.ldata = data.size();
    int l = event.simpleData.lchannel + event.simpleData.lmessage +
            event.simpleData.ldata;

    // combine channel, message and data into one block of raw bytes
    QByteArray raw(l, 0);
    char *d = (char*)raw.data();
    memcpy(d, ch.toLatin1().constData(), event.simpleData.lchannel);
    d += event.simpleData.lchannel;
    memcpy(d, msg.toLatin1().constData(), event.simpleData.lmessage);
    d += event.simpleData.lmessage;
    memcpy(d, data.data(), event.simpleData.ldata);

    event.setData(raw.data(), l);

    c->sendEvent(&event);
}
#endif

void QWSServerPrivate::sendRegionEvent(QWSWindow *window,
                                       const QRegion &region) const
{
    QWSRegionEvent event;
    event.setData(window->winId(), region, QWSRegionEvent::Request);
    window->client()->sendEvent(&event);
}

/*!
    \fn QWSWindow *QWSServer::windowAt(const QPoint& position)

    Returns the window containing the given \a position, returns 0 if
    there is no window under the specified point.

    \sa clientWindows()
*/
QWSWindow *QWSServer::windowAt(const QPoint& pos)
{
    Q_D(QWSServer);
    for (int i=0; i<d->windows.size(); ++i) {
        QWSWindow* w = d->windows.at(i);
        if (w->allocatedRegion().contains(pos))
            return w;
    }
    return 0;
}

#ifndef QT_NO_QWS_KEYBOARD
static int keyUnicode(int keycode)
{
    int code = 0xffff;

    if (keycode >= Qt::Key_A && keycode <= Qt::Key_Z)
        code = keycode - Qt::Key_A + 'a';
    else if (keycode >= Qt::Key_0 && keycode <= Qt::Key_9)
        code = keycode - Qt::Key_0 + '0';

    return code;
}
#endif

/*!
    Sends a key event. Note that this function is called by the
    processKeyEvent() function. Use this function to send key events
    generated by "virtual keyboards".

    \table
    \header \o Parameter \o Description
    \row
        \o \a unicode
        \o The unicode value of the key to send
    \row
        \o \a keycode
        \o The Qt keycode value as defined by the Qt::Key enum.
    \row
        \o \a modifiers
        \o An OR combination of Qt::KeyboardModifier values, indicating whether
            Shift/Alt/Ctrl keys are pressed.
    \row
        \o \a isPress
        \o True if this is a key down event; otherwise false.
    \row
        \o \a autoRepeat
        \o True if this event is caused by auto repeat (i.e. the
            user has held the key down and this is the second or subsequent
            key event being sent); otherwise false.
    \endtable

    \sa processKeyEvent(), {Qtopia Core Character Input}
*/
void QWSServer::sendKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                             bool isPress, bool autoRepeat)
{
    qws_keyModifiers = modifiers;

    if (isPress) {
        if (keycode != Qt::Key_F34 && keycode != Qt::Key_F35)
            qwsServerPrivate->_q_screenSaverWake();
    }

#ifndef QT_NO_QWS_INPUTMETHODS

    if (!current_IM || !current_IM->filter(unicode, keycode, modifiers, isPress, autoRepeat))
        QWSServerPrivate::sendKeyEventUnfiltered(unicode, keycode, modifiers, isPress, autoRepeat);
#else
    QWSServerPrivate::sendKeyEventUnfiltered(unicode, keycode, modifiers, isPress, autoRepeat);
#endif
}

void QWSServerPrivate::sendKeyEventUnfiltered(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                                       bool isPress, bool autoRepeat)
{

    QWSKeyEvent event;
    QWSWindow *win = keyboardGrabber ? keyboardGrabber :
        qwsServerPrivate->focusw;

    event.simpleData.window = win ? win->winId() : 0;

    event.simpleData.unicode =
#ifndef QT_NO_QWS_KEYBOARD
        unicode < 0 ? keyUnicode(keycode) :
#endif
        unicode;
    event.simpleData.keycode = keycode;
    event.simpleData.modifiers = modifiers;
    event.simpleData.is_press = isPress;
    event.simpleData.is_auto_repeat = autoRepeat;

    for (ClientIterator it = qwsServerPrivate->clientMap.begin(); it != qwsServerPrivate->clientMap.end(); ++it)
        (*it)->sendEvent(&event);
}

/*!
    \internal
*/
void QWSServer::beginDisplayReconfigure()
{
    qwsServer->enablePainting(false);
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->hide();
#endif
    QWSDisplay::grab(true);
    qt_screen->disconnect();
}

/*!
    \internal
*/
void QWSServer::endDisplayReconfigure()
{
    qt_screen->connect(QString());
    qwsServerPrivate->swidth = qt_screen->deviceWidth();
    qwsServerPrivate->sheight = qt_screen->deviceHeight();

    QWSDisplay::ungrab();
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->show();
#endif
    QApplicationPrivate *ap = QApplicationPrivate::instance();
    ap->setMaxWindowRect(qt_screen,
                         QRect(0, 0, qt_screen->deviceWidth(), qt_screen->deviceHeight()));
    QSize olds = qApp->desktop()->size();
    qApp->desktop()->resize(qt_screen->width(), qt_screen->height());
    qApp->postEvent(qApp->desktop(), new QResizeEvent(qApp->desktop()->size(), olds));
    qwsServer->enablePainting(true);
    qwsServer->refresh();
    qDebug("Desktop size: %dx%d", qApp->desktop()->width(), qApp->desktop()->height());
}

void QWSServerPrivate::resetEngine()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->hide();
    qt_screencursor->show();
#endif
}


#ifndef QT_NO_QWS_CURSOR
/*!
    \fn void QWSServer::setCursorVisible(bool visible)

    Makes the cursor visible if \a visible is true: otherwise the
    cursor is made invisible.

    \sa isCursorVisible()
*/
void QWSServer::setCursorVisible(bool vis)
{
    if (qwsServerPrivate && qwsServerPrivate->haveviscurs != vis) {
        QWSCursor* c = qwsServerPrivate->cursor;
        qwsServerPrivate->setCursor(QWSCursor::systemCursor(Qt::BlankCursor));
        qwsServerPrivate->haveviscurs = vis;
        qwsServerPrivate->setCursor(c);
    }
}

/*!
    Returns true if the cursor is visible; otherwise returns false.

    \sa setCursorVisible()
*/
bool QWSServer::isCursorVisible()
{
    return qwsServerPrivate ? qwsServerPrivate->haveviscurs : true;
}
#endif

#ifndef QT_NO_QWS_INPUTMETHODS


/*!
    \fn void QWSServer::sendIMEvent(const QInputMethodEvent *event)

    Sends the given input method \a event.

    If there is a window currently in compose mode (i.e. actively
    composing the preedit string ), the event is sent to that
    window. Otherwise, the event is sent to the current focus window.

    \sa sendIMQuery(), QWSInputMethod::sendEvent()
*/
void QWSServer::sendIMEvent(const QInputMethodEvent *ime)
{
    QWSIMEvent event;

    QWSWindow *win = keyboardGrabber ? keyboardGrabber :
                     qwsServerPrivate->focusw;

    //if currently composing then event must go to the composing window

    if (current_IM_composing_win)
        win = current_IM_composing_win;

    event.simpleData.window = win ? win->winId() : 0;
    event.simpleData.replaceFrom = ime->replacementStart();;
    event.simpleData.replaceLength = ime->replacementLength();

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);

    out << ime->preeditString();
    out << ime->commitString();

    const QList<QInputMethodEvent::Attribute> &attributes = ime->attributes();
    for (int i = 0; i < attributes.count(); ++i) {
        const QInputMethodEvent::Attribute &a = attributes.at(i);
        out << a.type << a.start << a.length << a.value;
    }
    event.setData(buffer.data(), buffer.size());
    QWSClient *serverClient = qwsServerPrivate->clientMap.value(-1);
    if (serverClient)
        serverClient->sendEvent(&event);
    if (win && win->client() && win->client() != serverClient)
        win->client()->sendEvent(&event);

    current_IM_composing_win = ime->preeditString().isEmpty() ? 0 : win;
    current_IM_winId = win->winId();
}


/*!
    Sends an input method query for the specified \a property.

    To receive responses to input method queries the virtual
    QWSInputMethod::queryResponse() function must be reimplemented in
    a QWSInputMethod subclass.

    \sa sendIMEvent() QWSInputMethod::sendQuery()
*/
void QWSServer::sendIMQuery(int property)
{
    QWSIMQueryEvent event;

    QWSWindow *win = keyboardGrabber ? keyboardGrabber :
                     qwsServerPrivate->focusw;
    if (current_IM_composing_win)
        win = current_IM_composing_win;

    event.simpleData.window = win ? win->winId() : 0;
    event.simpleData.property = property;
    if (win && win->client())
        win->client()->sendEvent(&event);
}



/*!
    \fn void QWSServer::setCurrentInputMethod(QWSInputMethod *method)

    Sets the current input method to be the given \a method.

    \sa sendIMQuery(), sendIMEvent(),  QWSInputMethod
*/
void QWSServer::setCurrentInputMethod(QWSInputMethod *im)
{
    if (current_IM)
        current_IM->reset(); //??? send an update event instead ?
    current_IM = im;
}

/*!
    \fn static void QWSServer::resetInputMethod()

    \internal
*/

#endif //QT_NO_QWS_INPUTMETHODS

#ifndef QT_NO_QWS_PROPERTIES
/*!
  \internal
*/
void QWSServer::sendPropertyNotifyEvent(int property, int state)
{
    Q_D(QWSServer);
    QWSServerPrivate::ClientIterator it = d->clientMap.begin();
    while (it != d->clientMap.end()) {
        QWSClient *cl = *it;
        ++it;
        cl->sendPropertyNotifyEvent(property, state);
    }
}
#endif

void QWSServerPrivate::invokeIdentify(const QWSIdentifyCommand *cmd, QWSClient *client)
{
    client->setIdentity(cmd->id);
#ifndef QT_NO_QWS_MULTIPROCESS
    if (client->clientId() > 0)
        client->d_func()->setLockId(cmd->simpleData.idLock);
#endif
}

void QWSServerPrivate::invokeCreate(QWSCreateCommand *cmd, QWSClient *client)
{
    QWSCreationEvent event;
    event.simpleData.objectid = get_object_id(cmd->count);
    event.simpleData.count = cmd->count;
    client->sendEvent(&event);
}

void QWSServerPrivate::invokeRegionName(const QWSRegionNameCommand *cmd, QWSClient *client)
{
    Q_Q(QWSServer);
    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, client);
    if (changingw) {
        changingw->setName(cmd->name);
        changingw->setCaption(cmd->caption);
        emit q->windowEvent(changingw, QWSServer::Name);
    }
}

void QWSServerPrivate::invokeRegion(QWSRegionCommand *cmd, QWSClient *client)
{
#ifdef QWS_REGION_DEBUG
    qDebug("QWSServer::invokeRegion %d rects (%d)",
            cmd->simpleData.nrectangles, cmd->simpleData.windowid);
#endif

    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, 0);
    if (!changingw) {
        qWarning("Invalid window handle %08x",cmd->simpleData.windowid);
        return;
    }
    if (!changingw->forClient(client)) {
        qWarning("Disabled: clients changing other client's window region");
        return;
    }

    request_region(cmd->simpleData.windowid, cmd->surfaceKey, cmd->surfaceData,
                   cmd->region);
}

void QWSServerPrivate::invokeRegionMove(const QWSRegionMoveCommand *cmd, QWSClient *client)
{
    Q_Q(QWSServer);
    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, 0);
    if (!changingw) {
        qWarning("invokeRegionMove: Invalid window handle %d",cmd->simpleData.windowid);
        return;
    }
    if (!changingw->forClient(client)) {
        qWarning("Disabled: clients changing other client's window region");
        return;
    }

//    changingw->setNeedAck(true);
    moveWindowRegion(changingw, cmd->simpleData.dx, cmd->simpleData.dy);
    emit q->windowEvent(changingw, QWSServer::Geometry);
}

void QWSServerPrivate::invokeRegionDestroy(const QWSRegionDestroyCommand *cmd, QWSClient *client)
{
    Q_Q(QWSServer);
    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, 0);
    if (!changingw) {
        qWarning("invokeRegionDestroy: Invalid window handle %d",cmd->simpleData.windowid);
        return;
    }
    if (!changingw->forClient(client)) {
        qWarning("Disabled: clients changing other client's window region");
        return;
    }

    setWindowRegion(changingw, QRegion());
//    rgnMan->remove(changingw->allocationIndex());
    for (int i = 0; i < windows.size(); ++i) {
        if (windows.at(i) == changingw) {
            windows.takeAt(i);
            if (i < nReserved)
                --nReserved;
            break;
        }
    }

    handleWindowClose(changingw);
#ifndef QT_NO_QWS_PROPERTIES
    propertyManager.removeProperties(changingw->winId());
#endif
    emit q->windowEvent(changingw, QWSServer::Destroy);
    delete changingw;
}

void QWSServerPrivate::invokeSetFocus(const QWSRequestFocusCommand *cmd, QWSClient *client)
{
    int winId = cmd->simpleData.windowid;
    int gain = cmd->simpleData.flag;

    if (gain != 0 && gain != 1) {
        qWarning("Only 0(lose) and 1(gain) supported");
        return;
    }

    QWSWindow* changingw = findWindow(winId, 0);
    if (!changingw)
        return;

    if (!changingw->forClient(client)) {
       qWarning("Disabled: clients changing other client's focus");
        return;
    }

    setFocus(changingw, gain);
}

void QWSServerPrivate::setFocus(QWSWindow* changingw, bool gain)
{
    Q_Q(QWSServer);
#ifndef QT_NO_QWS_INPUTMETHODS
    /*
      This is the logic:
      QWSWindow *loser = 0;
      if (gain && focusw != changingw)
         loser = focusw;
      else if (!gain && focusw == changingw)
         loser = focusw;
      But these five lines can be reduced to one:
    */
    if (current_IM) {
        QWSWindow *loser =  (!gain == (focusw==changingw)) ? focusw : 0;
        if (loser && loser->winId() == current_IM_winId)
            current_IM->updateHandler(QWSInputMethod::FocusOut);
    }
#endif
    if (gain) {
        if (focusw != changingw) {
            if (focusw) focusw->focus(0);
            focusw = changingw;
            focusw->focus(1);
            emit q->windowEvent(focusw, QWSServer::Active);
        }
    } else if (focusw == changingw) {
        if (changingw->client())
            changingw->focus(0);
        focusw = 0;
        // pass focus to window which most recently got it...
        QWSWindow* bestw=0;
        for (int i=0; i<windows.size(); ++i) {
            QWSWindow* w = windows.at(i);
            if (w != changingw && !w->hidden() &&
                    (!bestw || bestw->focusPriority() < w->focusPriority()))
                bestw = w;
        }
        if (!bestw && changingw->focusPriority()) { // accept focus back?
            bestw = changingw; // must be the only one
        }
        focusw = bestw;
        if (focusw) {
            focusw->focus(1);
            emit q->windowEvent(focusw, QWSServer::Active);
        }
    }
}



void QWSServerPrivate::invokeSetOpacity(const QWSSetOpacityCommand *cmd, QWSClient *client)
{
    Q_UNUSED( client );
    int winId = cmd->simpleData.windowid;
    int opacity = cmd->simpleData.opacity;

    QWSWindow* changingw = findWindow(winId, 0);

    if (!changingw) {
        qWarning("invokeSetOpacity: Invalid window handle %d", winId);
        return;
    }

    int altitude = windows.indexOf(changingw);
    const bool wasOpaque = changingw->isOpaque();
    changingw->_opacity = opacity;
    if (wasOpaque != changingw->isOpaque())
        update_regions();
    exposeRegion(changingw->allocatedRegion(), altitude);
}

void QWSServerPrivate::invokeSetAltitude(const QWSChangeAltitudeCommand *cmd,
                                   QWSClient *client)
{
    Q_UNUSED(client);

    int winId = cmd->simpleData.windowid;
    int alt = cmd->simpleData.altitude;
    bool fixed = cmd->simpleData.fixed;
#if 0
    qDebug("QWSServer::invokeSetAltitude winId %d alt %d)", winId, alt);
#endif

    if (alt < -1 || alt > 1) {
        qWarning("QWSServer::invokeSetAltitude Only lower, raise and stays-on-top supported");
        return;
    }

    QWSWindow* changingw = findWindow(winId, 0);
    if (!changingw) {
        qWarning("invokeSetAltitude: Invalid window handle %d", winId);
        return;
    }

    if (fixed && alt >= 1) {
        changingw->onTop = true;
    }
    if (alt == QWSChangeAltitudeCommand::Lower)
        changingw->lower();
    else
        changingw->raise();

//      if (!changingw->forClient(client)) {
//         refresh();
//     }
}

#ifndef QT_NO_QWS_PROPERTIES
void QWSServerPrivate::invokeAddProperty(QWSAddPropertyCommand *cmd)
{
    propertyManager.addProperty(cmd->simpleData.windowid, cmd->simpleData.property);
}

void QWSServerPrivate::invokeSetProperty(QWSSetPropertyCommand *cmd)
{
    Q_Q(QWSServer);
    if (propertyManager.setProperty(cmd->simpleData.windowid,
                                    cmd->simpleData.property,
                                    cmd->simpleData.mode,
                                    cmd->data,
                                    cmd->rawLen)) {
        q->sendPropertyNotifyEvent(cmd->simpleData.property,
                                 QWSPropertyNotifyEvent::PropertyNewValue);
#ifndef QT_NO_QWS_INPUTMETHODS
        if (cmd->simpleData.property == QT_QWS_PROPERTY_MARKEDTEXT) {
            QString s((const QChar*)cmd->data, cmd->rawLen/2);
            emit q->markedText(s);
        }
#endif
    }
}

void QWSServerPrivate::invokeRemoveProperty(QWSRemovePropertyCommand *cmd)
{
    Q_Q(QWSServer);
    if (propertyManager.removeProperty(cmd->simpleData.windowid,
                                       cmd->simpleData.property)) {
        q->sendPropertyNotifyEvent(cmd->simpleData.property,
                                 QWSPropertyNotifyEvent::PropertyDeleted);
    }
}

void QWSServerPrivate::invokeGetProperty(QWSGetPropertyCommand *cmd, QWSClient *client)
{
    const char *data;
    int len;

    if (propertyManager.getProperty(cmd->simpleData.windowid,
                                    cmd->simpleData.property,
                                    data, len)) {
        client->sendPropertyReplyEvent(cmd->simpleData.property, len, data);
    } else {
        client->sendPropertyReplyEvent(cmd->simpleData.property, -1, 0);
    }
}
#endif //QT_NO_QWS_PROPERTIES

void QWSServerPrivate::invokeSetSelectionOwner(QWSSetSelectionOwnerCommand *cmd)
{
    qDebug("QWSServer::invokeSetSelectionOwner");

    SelectionOwner so;
    so.windowid = cmd->simpleData.windowid;
    so.time.set(cmd->simpleData.hour, cmd->simpleData.minute,
                 cmd->simpleData.sec, cmd->simpleData.ms);

    if (selectionOwner.windowid != -1) {
        QWSWindow *win = findWindow(selectionOwner.windowid, 0);
        if (win)
            win->client()->sendSelectionClearEvent(selectionOwner.windowid);
        else
            qDebug("couldn't find window %d", selectionOwner.windowid);
    }

    selectionOwner = so;
}

void QWSServerPrivate::invokeConvertSelection(QWSConvertSelectionCommand *cmd)
{
    qDebug("QWSServer::invokeConvertSelection");

    if (selectionOwner.windowid != -1) {
        QWSWindow *win = findWindow(selectionOwner.windowid, 0);
        if (win)
            win->client()->sendSelectionRequestEvent(cmd, selectionOwner.windowid);
        else
            qDebug("couldn't find window %d", selectionOwner.windowid);
    }
}

#ifndef QT_NO_QWS_CURSOR
void QWSServerPrivate::invokeDefineCursor(QWSDefineCursorCommand *cmd, QWSClient *client)
{
    if (cmd->simpleData.height > 64 || cmd->simpleData.width > 64) {
        qDebug("Cannot define cursor size > 64x64");
        return;
    }

    delete client->cursors.take(cmd->simpleData.id);

    int dataLen = cmd->simpleData.height * ((cmd->simpleData.width+7) / 8);

    if (dataLen > 0 && cmd->data) {
        QWSCursor *curs = new QWSCursor(cmd->data, cmd->data + dataLen,
                                        cmd->simpleData.width, cmd->simpleData.height,
                                        cmd->simpleData.hotX, cmd->simpleData.hotY);
        client->cursors.insert(cmd->simpleData.id, curs);
    }
}

void QWSServerPrivate::invokeSelectCursor(QWSSelectCursorCommand *cmd, QWSClient *client)
{
    int id = cmd->simpleData.id;
    QWSCursor *curs = 0;
    if (id <= Qt::LastCursor) {
        curs = QWSCursor::systemCursor(id);
    }
    else {
        QWSCursorMap cursMap = client->cursors;
        QWSCursorMap::Iterator it = cursMap.find(id);
        if (it != cursMap.end()) {
            curs = it.value();
        }
    }
    if (curs == 0) {
        curs = QWSCursor::systemCursor(Qt::ArrowCursor);
    }

    QWSWindow* win = findWindow(cmd->simpleData.windowid, 0);
    if (mouseGrabber) {
        // If the mouse is being grabbed, we don't want just anyone to
        // be able to change the cursor.  We do want the cursor to be set
        // correctly once mouse grabbing is stopped though.
        if (win != mouseGrabber)
            nextCursor = curs;
        else
            setCursor(curs);
    } else if (win && win->allocatedRegion().contains(QWSServer::mousePosition)) { //##################### cursor
        // A non-grabbing window can only set the cursor shape if the
        // cursor is within its allocated region.
        setCursor(curs);
    }
}

void QWSServerPrivate::invokePositionCursor(QWSPositionCursorCommand *cmd, QWSClient *)
{
    Q_Q(QWSServer);
    QPoint newPos(cmd->simpleData.newX, cmd->simpleData.newY);
    if (newPos != QWSServer::mousePosition)
        q->sendMouseEvent(newPos, qwsServer->d_func()->mouseState);
}
#endif

void QWSServerPrivate::invokeGrabMouse(QWSGrabMouseCommand *cmd, QWSClient *client)
{
    QWSWindow* win = findWindow(cmd->simpleData.windowid, 0);
    if (!win)
        return;

    if (cmd->simpleData.grab) {
        if (!mouseGrabber || mouseGrabber->client() == client) {
            mouseGrabbing = true;
            mouseGrabber = win;
        }
    } else {
        releaseMouse(mouseGrabber);
    }
}

void QWSServerPrivate::invokeGrabKeyboard(QWSGrabKeyboardCommand *cmd, QWSClient *client)
{
    QWSWindow* win = findWindow(cmd->simpleData.windowid, 0);
    if (!win)
        return;

    if (cmd->simpleData.grab) {
        if (!keyboardGrabber || (keyboardGrabber->client() == client)) {
            keyboardGrabbing = true;
            keyboardGrabber = win;
        }
    } else {
        releaseKeyboard(keyboardGrabber);
    }
}

#if !defined(QT_NO_SOUND)
void QWSServerPrivate::invokePlaySound(QWSPlaySoundCommand *cmd, QWSClient *)
{
#if !defined(QT_EXTERNAL_SOUND_SERVER) && !defined(Q_OS_DARWIN)
    soundserver->playFile( 1, cmd->filename );
#else
    Q_UNUSED(cmd);
#endif
}
#endif

#ifndef QT_NO_COP
void QWSServerPrivate::invokeRegisterChannel(QWSQCopRegisterChannelCommand *cmd,
                                       QWSClient *client)
{
  // QCopChannel will force us to emit the newChannel signal if this channel
  // didn't already exist.
  QCopChannel::registerChannel(cmd->channel, client);
}

void QWSServerPrivate::invokeQCopSend(QWSQCopSendCommand *cmd, QWSClient *client)
{
    QCopChannel::answer(client, cmd->channel, cmd->message, cmd->data);
}

#endif

#ifndef QT_NO_QWS_INPUTMETHODS
void QWSServer::resetInputMethod()
{
    if (current_IM && qwsServer) {
      current_IM->reset();
    }
}

void QWSServerPrivate::invokeIMResponse(const QWSIMResponseCommand *cmd,
                                 QWSClient *)
{
    if (current_IM)
        current_IM->queryResponse(cmd->simpleData.property, cmd->result);
}

void QWSServerPrivate::invokeIMUpdate(const QWSIMUpdateCommand *cmd,
                                 QWSClient *)
{
    if (cmd->simpleData.type == QWSInputMethod::FocusIn)
        current_IM_winId = cmd->simpleData.windowid;

    if (current_IM && (current_IM_winId == cmd->simpleData.windowid || cmd->simpleData.windowid == -1))
        current_IM->updateHandler(cmd->simpleData.type);
}

#endif

void QWSServerPrivate::invokeRepaintRegion(QWSRepaintRegionCommand * cmd,
                                           QWSClient *)
{
    QRegion r;
    r.setRects(cmd->rectangles,cmd->simpleData.nrectangles);
    repaint_region(cmd->simpleData.windowid, cmd->simpleData.opaque, r);
}

#ifndef QT_NO_QWSEMBEDWIDGET
void QWSServerPrivate::invokeEmbed(QWSEmbedCommand *cmd, QWSClient *client)
{
    // Should find these two windows in a single loop
    QWSWindow *embedder = findWindow(cmd->simpleData.embedder, client);
    QWSWindow *embedded = findWindow(cmd->simpleData.embedded);

    if (!embedder) {
        qWarning("QWSServer: Embed command from window %i failed: No such id.",
                 static_cast<int>(cmd->simpleData.embedder));
        return;
    }

    if (!embedded) {
        qWarning("QWSServer: Embed command on window %i failed: No such id.",
                 static_cast<int>(cmd->simpleData.embedded));
        return;
    }

    switch (cmd->simpleData.type) {
    case QWSEmbedEvent::StartEmbed:
        embedder->startEmbed(embedded);
        break;
    case QWSEmbedEvent::StopEmbed:
        embedder->stopEmbed(embedded);
        break;
    case QWSEmbedEvent::Region:
        break;
    }

    embedded->client()->sendEmbedEvent(embedded->winId(),
                                       cmd->simpleData.type, cmd->region);
    update_regions();
}
#endif // QT_NO_QWSEMBEDWIDGET

QWSWindow* QWSServerPrivate::newWindow(int id, QWSClient* client)
{
    Q_Q(QWSServer);
    // Make a new window, put it on top.
    QWSWindow* w = new QWSWindow(id,client);

    // insert after "stays on top" windows
    bool added = false;
    for (int i = nReserved; i < windows.size(); ++i) {
        QWSWindow *win = windows.at(i);
        if (!win->onTop) {
            windows.insert(i, w);
            added = true;
            break;
        }
    }
    if (!added)
        windows.append(w);
    emit q->windowEvent(w, QWSServer::Create);
    return w;
}

QWSWindow* QWSServerPrivate::findWindow(int windowid, QWSClient* client)
{
    for (int i=0; i<windows.size(); ++i) {
        QWSWindow* w = windows.at(i);
        if (w->winId() == windowid)
            return w;
    }
    if (client)
        return newWindow(windowid,client);
    else
        return 0;
}

void QWSServerPrivate::raiseWindow(QWSWindow *changingw, int /*alt*/)
{
    Q_Q(QWSServer);
    if (changingw == windows.first())
        return;

    // Expose regions previously overlapped by transparent windows
    const QRegion bound = changingw->allocatedRegion();
    QRegion expose;
    int windowPos = 0;

    //change position in list:
    for (int i = 0; i < windows.size(); ++i) {
        QWSWindow *w = windows.at(i);
        if (w == changingw) {
            windowPos = i;
            windows.takeAt(i);
            break;
        }
        if (!w->isOpaque())
            expose += (w->allocatedRegion() & bound);
    }

    int newPos = -1;
    if (changingw->onTop) {
        windows.insert(nReserved, changingw);
        newPos = nReserved;
    } else {
        // insert after "stays on top" windows
        bool in = false;
        for (int i = nReserved; i < windows.size(); ++i) {
            QWSWindow *w = windows.at(i);
            if (!w->onTop) {
                windows.insert(i, changingw);
                in = true;
                newPos = i;
                break;
            }
        }
        if (!in) {
            windows.append(changingw);
            newPos = windows.size()-1;
        }
    }

    if (windowPos != newPos) {
        update_regions();
        if (!expose.isEmpty())
            exposeRegion(expose, newPos);
    }
    emit q->windowEvent(changingw, QWSServer::Raise);
}

void QWSServerPrivate::lowerWindow(QWSWindow *changingw, int /*alt*/)
{
    Q_Q(QWSServer);
    if (changingw == windows.last())
        return;

    int i = windows.indexOf(changingw);
    windows.move(i,windows.size()-1);

    const QRegion bound = changingw->allocatedRegion();

    update_regions();

    // Expose regions previously overlapped by transparent window
    if (!changingw->isOpaque()) {
        QRegion expose;
        for (int j = i; j < windows.size() - 1; ++j)
            expose += (windows.at(j)->allocatedRegion() & bound);
        if (!expose.isEmpty())
            exposeRegion(expose, i);
    }

    emit q->windowEvent(changingw, QWSServer::Lower);
}

void QWSServerPrivate::update_regions()
{
    static QRegion prevAvailable = QRect(0, 0, qt_screen->width(), qt_screen->height());
    QRegion available = QRect(0, 0, qt_screen->width(), qt_screen->height());
    QRegion reserved;

    QList<QWSWindow*> transparentWindows;
    QList<QRegion> transparentRegions;

    // XXX (try?)grab display lock

    for (int i = 0; i < windows.count(); ++i) {
        QWSWindow *w = windows.at(i);
        QRegion r = (w->requested_region & available);

#ifndef QT_NO_QWSEMBEDWIDGET
        // Subtract regions needed for embedded windows
        const int n = w->d->embedded.size();
        for (int i = 0; i < n; ++i)
            r -= w->d->embedded.at(i)->requested_region;

        // Limited to the embedder region
        if (w->d->embedder)
            r &= w->d->embedder->requested_region;
#endif // QT_NO_QWSEMBEDWIDGET

        QWSWindowSurface *surface = w->windowSurface();
        if (surface && (surface->isReserved() || !surface->isBuffered())) {
            available -= r;
            reserved += r;
        } else if (!w->isOpaque()) {
            transparentWindows.append(w);
            transparentRegions.append(r);
            continue;
        } else {
            available -= r;
        }

        if (r == w->allocatedRegion())
            continue;

        w->setAllocatedRegion(r);
        w->client()->sendRegionEvent(w->winId(), r,
                                     QWSRegionEvent::Allocation);
    }

    // Adjust transparent windows for nonbuffered regions
    for (int i = 0; i < transparentWindows.count(); ++i) {
        QWSWindow *w = transparentWindows.at(i);
        const QRegion r = transparentRegions.at(i) - reserved;

        if (r == w->allocatedRegion())
            continue;

        w->setAllocatedRegion(r);
        w->client()->sendRegionEvent(w->winId(), r,
                                     QWSRegionEvent::Allocation);
    }

    QRegion expose = (available - prevAvailable);
    prevAvailable = available;
    if (!expose.isEmpty())
        exposeRegion(expose);

    // XXX ungrab display lock
}

void QWSServerPrivate::moveWindowRegion(QWSWindow *changingw, int dx, int dy)
{
    if (!changingw)
        return;

    const QRegion oldRegion(changingw->allocatedRegion());
    changingw->requested_region.translate(dx, dy);
    update_regions();
    const QRegion newRegion(changingw->allocatedRegion());

    int idx = windows.indexOf(changingw);
    exposeRegion(oldRegion + newRegion, idx);
}

/*!
    Changes the requested region of window \a changingw to \a r
    If \a changingw is 0, the server's reserved region is changed.
*/
void QWSServerPrivate::setWindowRegion(QWSWindow* changingw, QRegion r)
{
    if (!changingw) {
        qWarning("Not implemented in this release");
        return;
    }

    if (changingw->requested_region == r)
        return;

    const QRegion oldRegion(changingw->allocatedRegion());
    changingw->requested_region = r;
    update_regions();
    const QRegion newRegion(changingw->allocatedRegion());

    int idx = windows.indexOf(changingw);
    exposeRegion(oldRegion - newRegion, idx);
}


void QWSServerPrivate::exposeRegion(QRegion r, int changing)
{
    static bool initial = true;
    if (initial) {
        r = QRect(0,0,qt_screen->width(), qt_screen->height());
        changing = 0;
        initial = false;
    }
    qt_screen->exposeRegion(r, qMax(nReserved, changing));
}

/*!
    Closes all pointer devices (specified by the QWS_MOUSE_PROTO
    environment variable) by deleting the associated mouse handlers.

    \sa openMouse(), mouseHandler()
*/
void QWSServer::closeMouse()
{
    Q_D(QWSServer);
    qDeleteAll(d->mousehandlers);
    d->mousehandlers.clear();
}

/*!
    Opens the mouse devices specified by the QWS_MOUSE_PROTO
    environment variable.

    \sa closeMouse(), mouseHandler()
*/
void QWSServer::openMouse()
{
    Q_D(QWSServer);
    QString mice = QString::fromLatin1(qgetenv("QWS_MOUSE_PROTO"));
#if defined(QT_QWS_CASSIOPEIA)
    if (mice.isEmpty())
        mice = QLatin1String("TPanel:/dev/tpanel");
#endif
    if (mice.isEmpty())
        mice = *defaultMouse();
    closeMouse();
    bool needviscurs = true;
    if (mice != QLatin1String("None")) {
        const QStringList mouse = mice.split(QLatin1Char(' '));
        for (int i = mouse.size() - 1; i >= 0; --i) {
            QWSMouseHandler *handler = d->newMouseHandler(mouse.at(i));
            setMouseHandler(handler);
            /* XXX handle mouse cursor visibility sensibly
               if (!h->inherits("QCalibratedMouseHandler"))
               needviscurs = true;
            */
        }
    }
#ifndef QT_NO_QWS_CURSOR
    setCursorVisible(needviscurs);
#else
    Q_UNUSED(needviscurs)
#endif
}

/*!
  Suspends mouse handling by suspending each registered mouse handler.

  \sa resumeMouse(), QWSMouseHandler::suspend(), QWS_MOUSE_PROTO
*/
void QWSServer::suspendMouse()
{
    Q_D(QWSServer);
    for (int i=0; i < d->mousehandlers.size(); ++i)
        d->mousehandlers.at(i)->suspend();
}

/*!
    Resumes mouse handling by reactivating each registered mouse handler.

    \sa suspendMouse(), QWSMouseHandler::resume(), QWS_MOUSE_PROTO
*/
void QWSServer::resumeMouse()
{
    Q_D(QWSServer);
    for (int i=0; i < d->mousehandlers.size(); ++i)
        d->mousehandlers.at(i)->resume();
}



QWSMouseHandler* QWSServerPrivate::newMouseHandler(const QString& spec)
{
    int c = spec.indexOf(QLatin1Char(':'));
    QString mouseProto;
    QString mouseDev;
    if (c >= 0) {
        mouseProto = spec.left(c);
        mouseDev = spec.mid(c+1);
    } else {
        mouseProto = spec;
    }

    int screen = -1;
    const QList<QRegExp> regexps = QList<QRegExp>()
                                   << QRegExp(":screen=(\\d+)\\b")
                                   << QRegExp("\\bscreen=(\\d+):");
    for (int i = 0; i < regexps.size(); ++i) {
        QRegExp regexp = regexps.at(i);
        if (regexp.indexIn(mouseDev) == -1)
            continue;
        screen = regexp.cap(1).toInt();
        mouseDev.remove(regexp.pos(0), regexp.matchedLength());
        break;
    }

    QWSMouseHandler *handler = 0;
    handler = QMouseDriverFactory::create(mouseProto, mouseDev);
    if (screen != -1)
        handler->setScreen(qt_screen->subScreens().at(screen));

    return handler;
}

#ifndef QT_NO_QWS_KEYBOARD

/*!
    Closes all the keyboard devices (specified by the QWS_KEYBOARD
    environment variable) by deleting the associated keyboard
    handlers.

    \sa openKeyboard(),  keyboardHandler()
*/
void QWSServer::closeKeyboard()
{
    Q_D(QWSServer);
    qDeleteAll(d->keyboardhandlers);
    d->keyboardhandlers.clear();
}

/*!
    Returns the primary keyboard handler.

    \sa openKeyboard()
*/
QWSKeyboardHandler* QWSServer::keyboardHandler()
{
    return qwsServerPrivate->keyboardhandlers.first();
}

/*!
    \fn void QWSServer::setKeyboardHandler(QWSKeyboardHandler* handler)

    Sets the primary keyboard handler to be the given \a handler.

    \l {Qtopia Core} provides several ready-made keyboard drivers, and
    custom drivers are typically added using Qt's plugin
    mechanism. See the {Qtopia Core Character Input} documentation for
    details.

    \sa keyboardHandler()
*/
void QWSServer::setKeyboardHandler(QWSKeyboardHandler* kh)
{
    if (!kh)
        return;
    qwsServerPrivate->keyboardhandlers.removeAll(kh);
    qwsServerPrivate->keyboardhandlers.prepend(kh);
}

/*!
    Opens the keyboard devices specified by the QWS_KEYBOARD
    environment variable.

    \sa closeKeyboard(), keyboardHandler()
*/
void QWSServer::openKeyboard()
{
    QString keyboards = QString::fromLatin1(qgetenv("QWS_KEYBOARD"));
#if defined(QT_QWS_CASSIOPEIA)
    if (keyboards.isEmpty())
        keyboards = QLatin1String("Buttons");
#endif
    if (keyboards.isEmpty())
        keyboards = *defaultKeyboard();

    closeKeyboard();
    if (keyboards == QLatin1String("None"))
        return;

    QString device;
    QString type;
    QStringList keyboard = keyboards.split(QLatin1Char(' '));
    for (int i = keyboard.size() - 1; i >= 0; --i) {
        const QString spec = keyboard.at(i);
        int colon=spec.indexOf(QLatin1Char(':'));
        if (colon>=0) {
            type = spec.left(colon);
            device = spec.mid(colon+1);
        } else {
            type = spec;
        }
        QWSKeyboardHandler *handler = QKbdDriverFactory::create(type, device);
        setKeyboardHandler(handler);
    }
}

#endif //QT_NO_QWS_KEYBOARD

QPoint QWSServer::mousePosition;
QBrush *QWSServerPrivate::bgBrush = 0;

void QWSServerPrivate::move_region(const QWSRegionMoveCommand *cmd)
{
    QWSClient *serverClient = clientMap.value(-1);
    invokeRegionMove(cmd, serverClient);
}

void QWSServerPrivate::set_altitude(const QWSChangeAltitudeCommand *cmd)
{
    QWSClient *serverClient = clientMap.value(-1);
    invokeSetAltitude(cmd, serverClient);
}

void QWSServerPrivate::set_opacity(const QWSSetOpacityCommand *cmd)
{
    QWSClient *serverClient = clientMap.value(-1);
    invokeSetOpacity(cmd, serverClient);
}


void QWSServerPrivate::request_focus(const QWSRequestFocusCommand *cmd)
{
    invokeSetFocus(cmd, clientMap.value(-1));
}

void QWSServerPrivate::set_identity(const QWSIdentifyCommand *cmd)
{
    invokeIdentify(cmd, clientMap.value(-1));
}

void QWSServerPrivate::repaint_region(int wid, bool opaque, QRegion region)
{
    QWSWindow* changingw = findWindow(wid, 0);
    if (!changingw) {
        return;
    }

    const bool isOpaque = changingw->opaque;
    changingw->opaque = opaque;
    if (isOpaque != opaque)
        update_regions();

    int level = windows.indexOf(changingw);
    exposeRegion(region, level);
}

QRegion QWSServerPrivate::reserve_region(QWSWindow *win, const QRegion &region)
{
    QRegion r = region;

    int oldPos = windows.indexOf(win);
    int newPos = oldPos < nReserved ? nReserved - 1 : nReserved;
    for (int i = 0; i < nReserved; ++i) {
        if (i != oldPos) {
            QWSWindow *w = windows.at(i);
            r -= w->requested_region;
        }
    }
    windows.move(oldPos, newPos);
    nReserved = newPos + 1;
    win->client()->sendRegionEvent(win->winId(), r, 0);

    return r;
}

void QWSServerPrivate::request_region(int wid, const QString &surfaceKey,
                                      const QByteArray &surfaceData,
                                      const QRegion &region)
{
    QWSWindow *changingw = findWindow(wid, 0);
    if (!changingw)
        return;

    changingw->createSurface(surfaceKey, surfaceData);
    QWSWindowSurface *surface = changingw->windowSurface();

    changingw->opaque = surface->isOpaque();

    QRegion r;
    if (surface->isReserved())
        r = reserve_region(changingw, region);
    else
        r = region;

    bool isShow = !changingw->isVisible() && !region.isEmpty();

    setWindowRegion(changingw, r);

    Q_Q(QWSServer);

    if (isShow)
        emit q->windowEvent(changingw, QWSServer::Show);
    if (!region.isEmpty())
        emit q->windowEvent(changingw, QWSServer::Geometry);
    else
        emit q->windowEvent(changingw, QWSServer::Hide);
    if (region.isEmpty())
        handleWindowClose(changingw);
}

void QWSServerPrivate::destroy_region(const QWSRegionDestroyCommand *cmd)
{
    invokeRegionDestroy(cmd, clientMap.value(-1));
}

void QWSServerPrivate::name_region(const QWSRegionNameCommand *cmd)
{
    invokeRegionName(cmd, clientMap.value(-1));
}

#ifndef QT_NO_QWS_INPUTMETHODS
void QWSServerPrivate::im_response(const QWSIMResponseCommand *cmd)
 {
     invokeIMResponse(cmd, clientMap.value(-1));
}

void QWSServerPrivate::im_update(const QWSIMUpdateCommand *cmd)
{
    invokeIMUpdate(cmd, clientMap.value(-1));
}

void QWSServerPrivate::send_im_mouse(const QWSIMMouseCommand *cmd)
{
    if (current_IM)
        current_IM->mouseHandler(cmd->simpleData.index, cmd->simpleData.state);
}
#endif

void QWSServerPrivate::openDisplay()
{
    qt_init_display();

//    rgnMan = qt_fbdpy->regionManager();
    swidth = qt_screen->deviceWidth();
    sheight = qt_screen->deviceHeight();
}

void QWSServerPrivate::closeDisplay()
{
    qt_screen->shutdownDevice();
}

/*!
    Returns the brush used as background in the absence of obscuring
    windows.

    \sa setBackground()
*/
const QBrush &QWSServer::backgroundBrush() const
{
    return *QWSServerPrivate::bgBrush;
}

/*!
    Sets the brush used as background in the absence of obscuring
    windows, to be the given \a brush.

    \sa backgroundBrush()
*/
void QWSServer::setBackground(const QBrush &brush)
{
    if (!QWSServerPrivate::bgBrush)
        QWSServerPrivate::bgBrush = new QBrush(brush);
    else
        *QWSServerPrivate::bgBrush = brush;
    if (!qwsServer)
        return;
    qt_screen->exposeRegion(QRect(0,0,qt_screen->width(), qt_screen->height()), 0);
}


#ifdef QT3_SUPPORT
/*!
    Sets the image \a img to be used as the background in the absence
    of obscuring windows.
*/
void QWSServer::setDesktopBackground(const QImage &img)
{
    if (img.isNull())
        setBackground(Qt::NoBrush);
    else
        setBackground(QBrush(QPixmap::fromImage(img)));
}

/*!
    \overload

    Sets the color \a c to be used as the background in the absence of
    obscuring windows.
*/
void QWSServer::setDesktopBackground(const QColor &c)
{
    setDesktopBackground(QBrush(c));
}
#endif //QT3_SUPPORT

/*!
  \internal
 */
void QWSServer::startup(int flags)
{
    if (qwsServer)
        return;
    unlink(qws_qtePipeFilename().toLatin1().constData());
    (void)new QWSServer(flags);
}

/*!
  \internal
*/

void QWSServer::closedown()
{
    unlink(qws_qtePipeFilename().toLatin1().constData());
    delete qwsServer;
    qwsServer = 0;
}

void QWSServerPrivate::emergency_cleanup()
{
#ifndef QT_NO_QWS_KEYBOARD
    if (qwsServer)
        qwsServer->closeKeyboard();
#endif
}

#ifndef QT_NO_QWS_KEYBOARD
static QList<QWSServer::KeyboardFilter*> *keyFilters = 0;

/*!
    Processes the key event characterized by the given parameters:

    \table
    \header \o Parameter \o Description
    \row
        \o \a unicode
        \o The unicode value of the key to send
    \row
        \o \a keycode
        \o The Qt keycode value as defined by the Qt::Key enum.
    \row
        \o \a modifiers
        \o An OR combination of Qt::KeyboardModifier values, indicating whether
            Shift/Alt/Ctrl keys are pressed.
    \row
        \o \a isPress
        \o True if this is a key down event; otherwise false.
    \row
        \o \a autoRepeat
        \o True if this event is caused by auto repeat (i.e. the
            user has held the key down and this is the second or subsequent
            key event being sent); otherwise false.
    \endtable

    This function is typically called internally by keyboard drivers.

    \sa sendKeyEvent(), addKeyboardFilter(), {Qtopia Core Character Input}
*/
void QWSServer::processKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                                bool isPress, bool autoRepeat)
{
    if (keyFilters) {
        for (int i = 0; i < keyFilters->size(); ++i) {
            QWSServer::KeyboardFilter *keyFilter = keyFilters->at(i);
            if (keyFilter->filter(unicode, keycode, modifiers, isPress, autoRepeat))
                return;
        }
    }
    sendKeyEvent(unicode, keycode, modifiers, isPress, autoRepeat);
}

/*!
    \fn void QWSServer::addKeyboardFilter(KeyboardFilter *filter)

    Makes the given \a filter the one invoked for all key events
    generated by physical keyboard drivers (i.e. events sent using the
    processKeyEvent() function).

    Note that the filter is not invoked for keys generated by \e
    virtual keyboard drivers (i.e. events sent using the
    sendKeyEvent() function).

    \sa removeKeyboardFilter()
*/
void QWSServer::addKeyboardFilter(KeyboardFilter *f)
{
     if (!keyFilters)
        keyFilters = new QList<QWSServer::KeyboardFilter*>;
     if (f) {
        keyFilters->prepend(f);
     }
}

/*
//#######
 We should probably obsolete the whole keyboard filter thing since
 it's not useful for input methods anyway

 We could do removeKeyboardFilter(KeyboardFilter *f), but
 the "remove and delete the filter" concept does not match "user
 remembers the pointer".
*/

/*!
    Removes and deletes the most recently added filter.

    Note that the programmer is responsible for matching each addition
    of a keyboard filter with a corresponding removal.

    \sa addKeyboardFilter()
*/
void QWSServer::removeKeyboardFilter()
{
     if (!keyFilters || keyFilters->isEmpty())
         return;
     delete keyFilters->takeAt(0);
}
#endif // QT_NO_QWS_KEYBOARD

/*!
    \fn void QWSServer::setScreenSaverIntervals(int* intervals)

    Sets a list of timeout \a intervals (specified in millisecond) for
    the screensaver. An interval of 0 milliseconds turns off the
    screensaver.

    Note that the array must be 0-terminated.

    \sa setScreenSaverInterval()
*/
void QWSServer::setScreenSaverIntervals(int* ms)
{
    if (!qwsServerPrivate)
        return;
    delete [] qwsServerPrivate->screensaverintervals;
    if (ms) {
        int* t=ms;
        int n=0;
        while (*t++) n++;
        if (n) {
            n++; // the 0
            qwsServerPrivate->screensaverintervals = new int[n];
            memcpy(qwsServerPrivate->screensaverintervals, ms, n*sizeof(int));
        } else {
            qwsServerPrivate->screensaverintervals = 0;
        }
    } else {
        qwsServerPrivate->screensaverintervals = 0;
    }
    qwsServerPrivate->screensaverinterval = 0;

    qwsServerPrivate->screensavertimer->stop();
    qt_screen->blank(false);
    qwsServerPrivate->_q_screenSaverWake();
}

/*!
    \fn void QWSServer::setScreenSaverInterval(int milliseconds)

    Sets the timeout interval for the screensaver to be the specified
    \a milliseconds. To turn off the screensaver, set the timout
    interval to 0.

    \sa setScreenSaverIntervals()
*/
void QWSServer::setScreenSaverInterval(int ms)
{
    int v[2];
    v[0] = ms;
    v[1] = 0;
    setScreenSaverIntervals(v);
}

extern bool qt_disable_lowpriority_timers; //in qeventloop_unix.cpp

void QWSServerPrivate::_q_screenSaverWake()
{
    if (screensaverintervals) {
        if (screensaverinterval != screensaverintervals) {
            if (saver) saver->restore();
            screensaverinterval = screensaverintervals;
        } else {
            if (!screensavertimer->isActive()) {
                qt_screen->blank(false);
                if (saver) saver->restore();
            }
        }
        screensavertimer->start(*screensaverinterval);
        screensavertime.start();
    }
    qt_disable_lowpriority_timers=false;
}

void QWSServerPrivate::_q_screenSaverSleep()
{
    qt_screen->blank(true);
#if !defined(QT_QWS_IPAQ) && !defined(QT_QWS_EBX)
    screensavertimer->stop();
#else
    if (screensaverinterval) {
        screensavertimer->start(*screensaverinterval);
        screensavertime.start();
    } else {
        screensavertimer->stop();
    }
#endif
    qt_disable_lowpriority_timers=true;
}

/*!
    \fn void QWSServer::setScreenSaver(QWSScreenSaver* screenSaver)

    Deletes the current screensaver and installs the given \a
    screenSaver instead.

    \sa screenSaverActivate()
*/
void QWSServer::setScreenSaver(QWSScreenSaver* ss)
{
    QWSServerPrivate *qd = qwsServer->d_func();
    delete qd->saver;
    qd->saver = ss;
}

void QWSServerPrivate::screenSave(int level)
{
    if (saver) {
        if (saver->save(level)) {
            if (screensaverinterval && screensaverinterval[1]) {
                screensavertimer->start(*++screensaverinterval);
                screensavertime.start();
            } else {
                screensaverinterval = 0;
            }
        } else {
            // for some reason, the saver don't want us to change to the
            // next level, so we'll stay at this level for another interval
            if (screensaverinterval && *screensaverinterval) {
                screensavertimer->start(*screensaverinterval);
                screensavertime.start();
            }
        }
    } else {
        screensaverinterval = 0;//screensaverintervals;
        _q_screenSaverSleep();
    }
}

void QWSServerPrivate::_q_screenSaverTimeout()
{
    if (screensaverinterval) {
        if (screensavertime.elapsed() > *screensaverinterval*2) {
            // bogus (eg. unsuspend, system time changed)
            _q_screenSaverWake(); // try again
            return;
        }
        screenSave(screensaverinterval - screensaverintervals);
    }
}

/*!
    Returns true if the screensaver is active (i.e. the screen is
    blanked); otherwise returns false.

    \sa screenSaverActivate()
*/
bool QWSServer::screenSaverActive()
{
    return qwsServerPrivate->screensaverinterval
        && !qwsServerPrivate->screensavertimer->isActive();
}

/*!
    Activates the screensaver immediately if the \a activate is true;
    otherwise it is deactivated.

    \sa screenSaverActive()
*/
void QWSServer::screenSaverActivate(bool activate)
{
    if (activate)
        qwsServerPrivate->_q_screenSaverSleep();
    else
        qwsServerPrivate->_q_screenSaverWake();
}

void QWSServerPrivate::disconnectClient(QWSClient *c)
{
    QTimer::singleShot(0, c, SLOT(closeHandler()));
}

void QWSServerPrivate::updateClientCursorPos()
{
    Q_Q(QWSServer);
    QWSWindow *win = qwsServerPrivate->mouseGrabber ? qwsServerPrivate->mouseGrabber : qwsServer->windowAt(QWSServer::mousePosition);
    QWSClient *winClient = win ? win->client() : 0;
    if (winClient && winClient != cursorClient)
        q->sendMouseEvent(QWSServer::mousePosition, mouseState);
}

#ifndef QT_NO_QWS_INPUTMETHODS

/*!
    \class QWSInputMethod
    \ingroup qws

    \brief The QWSInputMethod class provides international input methods
    in Qtopia Core.

    Note that this class is still subject to change.

    An input method consists of a keyboard filter and optionally a
    graphical interface. Derive from this class to implement custom
    input methods, and use the QWSServer::setCurrentInputMethod()
    function to install it.

    To intercept key events from physical or virtual keyboards,
    reimplement the filter() function and use the setInputResolution()
    function to control the number of bits shifted when going from
    pointer resolution to screen resolution in the process of
    filtering mouse events (the current settings can be retrieved
    using the inputResolutionShift() function). To handle mouse events
    within a preedit text, reimplement the virtual mouseHandler()
    function.

    Internally, preedit texts are passed encapsulated by an \l
    {QWSEvent::IMEvent}{IMEvent} generated by the sendPreeditString()
    function which is a convenience function for sendEvent(). The
    sendEvent() function sends a QInputMethodEvent object to the focus
    widget. QWSInputMethod also provides the sendCommitString()
    convenience function which sends a QInputMethodEvent object
    encapsulating given commit string to the focus widget, and the
    sendMouseEvent() function which creates (and sends) a QWSEvent of
    the QWSEvent::Mouse type from the given parameters.

    The reset() function allows subclasses of QWSInputMethod to reset
    the state of the input method. The default implementation calls
    sendEvent() with empty preedit and commit strings, if the input
    method is in compose mode, i.e. the input method is actively
    composing a preedit string.

    The QWSInputMethod class also provides the sendQuery() function
    for sending input method queries. When deriving from
    QWSInputMethod, the virtual queryResponse() function must be
    reimplemented to receive responses to such queries.

    Finally, QWSInputMethod provides the updateHandler() function
    handling update events, including resets and focus changes. The
    UpdateType enum describes the various types of update events
    recognized by the input method.

    \sa QWSEvent, QWSServer, {Qtopia Core}
*/

/*!
    Constructs a new input method.
*/

QWSInputMethod::QWSInputMethod()
{

}

/*!
    Destructs this input method, uninstalling it if it is installed.
*/
QWSInputMethod::~QWSInputMethod()
{
    if (current_IM == this)
        current_IM = 0;
}

/*!
    This virtual function allows subclasses of QWSInputMethod to
    handle key input from physical or virtual keyboards.

    To block the event from further processing, return true when
    reimplementing this function; the default implementation returns
    false.

    The key input is identified by its \a unicode, \a keycode (a
    Qt::Key value), \a modifiers (an OR combination of
    Qt::KeyboardModifiers), \a isPress telling whether the input is a
    key press or key release, and \a autoRepeat determining if the
    input is \l {QWSKeyboardHandler::beginAutoRepeat()}{auto
    repeated}.

    \sa QWSKeyboardHandler, {Qtopia Core Character Input}
*/
bool QWSInputMethod::filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat)
{
    Q_UNUSED(unicode);
    Q_UNUSED(keycode);
    Q_UNUSED(modifiers);
    Q_UNUSED(isPress);
    Q_UNUSED(autoRepeat);
    return false;
}

/*!
    \overload

    This virtual function allows subclasses of QWSInputMethod to
    handle mouse input from physical or virtual pointer devices.

    To block the event from further processing, return true when
    reimplementing this function; the default implementation returns
    false. The mouse event is specified by the given \a position, \a
    state and \a wheel parameters.

    \sa setInputResolution() QWSMouseHandler, {Qtopia Core Pointer
    Handling}
*/
bool QWSInputMethod::filter(const QPoint &position, int state, int wheel)
{
    Q_UNUSED(position);
    Q_UNUSED(state);
    Q_UNUSED(wheel);
    return false;
}

/*!
    This virtual function allows subclasses of QWSInputMethod to reset
    the state of the input method.

    The default implementation calls sendEvent() with empty preedit
    and commit strings, if the input method is in compose mode,
    i.e. the input method is actively composing a preedit string.

    \sa sendEvent()
*/
void QWSInputMethod::reset()
{
    if (current_IM_composing_win) {
        QInputMethodEvent ime;
        sendEvent(&ime);
    }
}

/*!
    \enum QWSInputMethod::UpdateType

    This enum describes the various types of update events recognized
    by the input method.

    \value Update    The input widget is updated in some way; use sendQuery() with
                            Qt::ImMicroFocus as an argument for more information.
    \value FocusIn   A new input widget receives focus.
    \value FocusOut  The input widget loses focus.
    \value Reset       The input method should be reset.
    \value Destroyed The input widget is destroyed.

    \sa updateHandler()
*/

/*!
    Handles update events including resets and focus changes.

    The update events are specified by the given \a type which is one
    of the UpdateType enum values. Note that reimplementations of this
    function must call the base implementation for all cases that it
    does not handle itself.

    \sa UpdateType
*/
void QWSInputMethod::updateHandler(int type)
{
    switch (type) {
    case FocusOut:
    case Reset:
        reset();
        break;

    default:
        break;
    }
}


/*!
    This virtual function allows subclasses of QWSInputMethod to
    receive replies to an input method query.

    Internally, an input method query is passed encapsulated by an \l
    {QWSEvent::IMQuery}{IMQuery} event generated by the sendQuery()
    function. The queried property and the result is passed in the \a
    property and \a result parameters.

    \sa sendQuery(), QWSServer::sendIMQuery()
*/
void QWSInputMethod::queryResponse(int property, const QVariant &result)
{
    Q_UNUSED(property);
    Q_UNUSED(result);
}



/*!
  \fn void QWSInputMethod::mouseHandler(int offset, int state)

  This virtual function allows subclasses of QWSInputMethod to handle
  mouse events within the preedit text.

  The \a offset parameter specifies the position of the mouse event
  within the string, and \a state the type of the mouse event as
  described by the QWSServer::IMMouse enum. If \a state is less than
  0, the mouse event is inside the associated widget, but outside the
  preedit text. When clicking in a different widget, the \a state is
  QWSServer::MouseOutside.

  The default implementation resets the input method on all mouse
  presses.

  \sa sendPreeditString(), reset()
*/
void QWSInputMethod::mouseHandler(int, int state)
{
    if (state == QWSServer::MousePress || state == QWSServer::MouseOutside)
        reset();
}


/*!
    Sends a QInputMethodEvent object encapsulating the given \a
    preeditString, to the focus widget. This is a convenience function
    for the sendEvent() function.

    The specified \a selectionLength is the number of characters to be
    marked as selected (starting at the given \a cursorPosition). If
    \a selectionLength is negative, the text \e before \a
    cursorPosition is marked.

    The preedit string is marked with QInputContext::PreeditFormat,
    and the selected part is marked with
    QInputContext::SelectionFormat.

    Sending an input method event with a non-empty preedit string will
    cause the input method to enter compose mode.  Sending an input
    method event with an empty preedit string will cause the input
    method to leave compose mode, i.e. the input method will no longer
    be actively composing the preedit string.

    \sa sendEvent(), sendCommitString()
*/

void QWSInputMethod::sendPreeditString(const QString &preeditString, int cursorPosition, int selectionLength)
{
    QList<QInputMethodEvent::Attribute> attributes;

    int selPos = cursorPosition;
    if (selectionLength == 0) {
        selPos = 0;
    } else if (selectionLength < 0) {
        selPos += selectionLength;
        selectionLength = -selectionLength;
    }
    if (selPos > 0)
        attributes += QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, selPos,
                                                   QVariant(int(QInputContext::PreeditFormat)));

    if (selectionLength)
        attributes += QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, selPos, selectionLength,
                                                   QVariant(int(QInputContext::SelectionFormat)));

    if (selPos + selectionLength < preeditString.length())
        attributes += QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat,
                                                   selPos + selectionLength,
                                                   preeditString.length() - selPos - selectionLength,
                                                   QVariant(int(QInputContext::PreeditFormat)));

    attributes += QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, cursorPosition,  0, QVariant());

    QInputMethodEvent ime(preeditString, attributes);
    qwsServer->sendIMEvent(&ime);
}

/*!
    \fn void QWSInputMethod::sendCommitString(const QString &commitString, int replaceFromPosition, int replaceLength)

    Sends a QInputMethodEvent object encapsulating the given \a
    commitString, to the focus widget. This is a convenience function
    for the sendEvent() function.

    If the specified \a replaceLength is greater than 0, the commit
    string will replace the given number of characters of the
    receiving widget's previous text, starting at the given \a
    replaceFromPosition relative to the start of the current preedit
    string.

    This will cause the input method to leave compose mode, i.e. the
    input method will no longer be actively composing the preedit
    string.

    \sa sendEvent(), sendPreeditString()
*/
void QWSInputMethod::sendCommitString(const QString &commitString, int replaceFrom, int replaceLength)
{
    QInputMethodEvent ime;
    ime.setCommitString(commitString, replaceFrom, replaceLength);
    qwsServer->sendIMEvent(&ime);
}

/*!
    \fn QWSInputMethod::sendIMEvent(QWSServer::IMState state, const QString &text, int cursorPosition, int selectionLength)
    \obsolete

    Sends a QInputMethodEvent object to the focus widget.

    If the specified \a state is QWSServer::IMCompose, \a text is a
    preedit string, \a cursorPosition is the cursor's position within
    the preedit string, and \a selectionLength is the number of
    characters (starting at \a cursorPosition) that should be marked
    as selected by the input widget receiving the event. If the
    specified \a state is QWSServer::IMEnd, \a text is a commit
    string.

    Use sendEvent(), sendPreeditString() or sendCommitString() instead.
*/

/*!
    \fn QWSInputMethod::sendEvent(const QInputMethodEvent *event)

    Sends the given \a event to the focus widget.

    \sa sendPreeditString(), sendCommitString(), reset()
*/


/*!
    \fn void QWSInputMethod::sendQuery(int property)

    Sends an input method query (internally encapsulated by a QWSEvent
    of the \l {QWSEvent::IMQuery}{IMQuery} type) for the specified \a
    property.

    To receive responses to input method queries, reimplement the
    virtual queryResponse() function.

    \sa queryResponse(), QWSServer::sendIMQuery()
*/

/*!
    Sets and returns the number of bits shifted to go from pointer
    resolution to screen resolution when filtering mouse events using
    the filter() function.

    If \a isHigh is true and the device has a pointer device
    resolution twice or more of the screen resolution, the positions
    passed to the filter() function will be presented at the higher
    resolution; otherwise the resolution will be equal to that of the
    screen resolution.

    \sa filter(), inputResolutionShift()
*/
uint QWSInputMethod::setInputResolution(bool isHigh)
{
    mIResolution = isHigh;
    return inputResolutionShift();
}

/*!
    Returns the number of bits shifted to go from pointer resolution
    to screen resolution when filtering mouse events using the
    filter() function.

    \sa setInputResolution()
*/
uint QWSInputMethod::inputResolutionShift() const
{
    return 0; // default for devices with single resolution.
}

/*!
    \fn void QWSInputMethod::sendMouseEvent( const QPoint &position, int state, int wheel )

    Sends a mouse event specified by the given \a position, \a state
    and \a wheel parameters.

    The given \a position will be transformed if the screen
    coordinates do not match the pointer device coordinates.

    Note that the event will be not be tested by the active input
    method, but calling the QWSServer::sendMouseEvent() function will
    make the current input method filter the event.

    \sa mouseHandler(), sendEvent()
*/
void QWSInputMethod::sendMouseEvent( const QPoint &pos, int state, int wheel )
{
        if (qt_last_x) {
         *qt_last_x = pos.x();
         *qt_last_y = pos.y();
    }
    QWSServer::mousePosition = pos;
    qwsServerPrivate->mouseState = state;
    QWSServerPrivate::sendMouseEventUnfiltered(pos, state, wheel);
}
#endif // QT_NO_QWS_INPUTMETHODS

/*!
    \fn  QWSWindow::QWSWindow(int i, QWSClient * client)
    \internal

    Constructs a new top-level window, associated with the client \a
    client and giving it the id \a i.
*/

/*!
    \fn QWSServer::windowEvent(QWSWindow * window, QWSServer::WindowEvent eventType)

    This signal is emitted whenever something happens to a top-level
    window (e.g. it's created or destroyed).

    The window to which the event has occurred and the event's type
    are passed in the \a window and \a eventType parameters,
    respectively.
*/

/*!
    \class QWSServer::KeyboardFilter
    \ingroup qws

    \brief The KeyboardFilter class is a base class for global
    keyboard event filters.

    Note that this class is only available in \l {Qtopia Core}.

    In \l {Qtopia Core}, all system generated events, including
    keyboard events, are passed to the server application which then
    propagates the event to the appropiate client. The KeyboardFilter
    class is used to implement a global, low-level filter on the
    server side. The server applies the filter to all keyboard events
    before passing them on to the clients:

    \image qwsserver_keyboardfilter.png

    This feature can, for example, be used to filter things like APM
    (advanced power management) suspended from a button without having
    to filter for it in all applications.

    To add a new keyboard filter you must first create the filter by
    deriving from this class, reimplementing the pure virtual filter()
    function. Then you can install the filter on the server using
    QWSServer's \l {QWSServer::}{addKeyboardFilter()}
    function. QWSServer also provides a \l
    {QWSServer::}{removeKeyboardFilter()} function.

    \sa {Qtopia Core Architecture}, QWSServer, QWSInputMethod
*/

/*!
    \fn QWSServer::KeyboardFilter::~KeyboardFilter()

    Destroys the keyboard filter.
*/

/*!
    \fn bool QWSServer::KeyboardFilter::filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat)

    Implement this function to return true if a given key event should
    be stopped from being processed any further; otherwise it should
    return false.

    A key event can be identified by the given \a unicode value and
    the \a keycode, \a modifiers, \a isPress and \a autoRepeat
    parameters.

    The \a keycode parameter is the Qt keycode value as defined by the
    Qt::Key enum. The \a modifiers is an OR combination of
    Qt::KeyboardModifier values, indicating whether \gui
    Shift/Alt/Ctrl keys are pressed. The \a isPress parameter is true
    if the event is a key press event and \a autoRepeat is true if the
    event is caused by an auto-repeat mechanism and not an actual key
    press.
*/

#include "moc_qwindowsystem_qws.cpp"
