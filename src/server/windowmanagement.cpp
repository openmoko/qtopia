/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "windowmanagement.h"
#include <QSet>
#include "qtopiaserverapplication.h"
#include <QWidget>
#include <qwindowsystem_qws.h>
#include <QApplication>
#include <QDesktopWidget>
#include <qscreen_qws.h>
#include <QRect>
#include <QSize>
#include <QTimer>
#include <QVector>
#include <QDebug>
#include <qvaluespace.h>
#include <qtopialog.h>

// declare ServerLayoutManager
class ServerLayoutManager : public QObject
{
    Q_OBJECT
public:
    ServerLayoutManager();

    void addDocked( QWidget *w, WindowManagement::DockArea placement, const QSize &s, int screen );

protected:
    virtual void customEvent(QEvent *);

private slots:
    void removeWidget();

private:
    struct Item {
        QWidget *w;
        WindowManagement::DockArea p;
        QSize fixed;
        int screen;
    };

    bool eventFilter( QObject *object, QEvent *event );
    void dolayout(int screen);
    void layout(int screen);
    Item *findWidget( const QWidget *w ) const;

    QVector<QList<Item*> > docked;
};

// define ServerLayoutManager
ServerLayoutManager::ServerLayoutManager()
    : docked(QApplication::desktop()->numScreens())
{
    qApp->desktop()->installEventFilter( this );
}

void ServerLayoutManager::addDocked( QWidget *w, WindowManagement::DockArea placement, const QSize &s, int screen )
{
    if (screen == -1)
        screen = QApplication::desktop()->primaryScreen();

    Item *oldItem = findWidget(w);
    if (oldItem) {
        docked[screen].removeAll(oldItem);
        delete oldItem;
    }

    Item *i = new Item;
    i->w = w;
    i->p = placement;
    i->fixed = s;
    i->screen = screen;
    if (!oldItem) {
        w->installEventFilter( this );
        connect(w, SIGNAL(destroyed()), this, SLOT(removeWidget()));
    }
    docked[screen].append(i);
    layout(screen);
}

void ServerLayoutManager::removeWidget()
{
    QWidget *w = (QWidget*)sender();
    Item *item = findWidget(w);
    if ( item ) {
        int screen = item->screen;
        docked[screen].removeAll( item );
        delete item;
        layout(screen);
    }
}

bool ServerLayoutManager::eventFilter( QObject *object, QEvent *event )
{
    if ( object == qApp->desktop() ) {
        if ( event->type() == QEvent::Resize ) {
            for (int screen = 0; screen < docked.count(); ++screen)
                layout(screen);
        }
        return QObject::eventFilter( object, event );
    }

    Item *item;

    switch ( event->type() ) {
        case QEvent::Hide:
        case QEvent::Show:
            item = findWidget( (QWidget *)object );
            if ( item )
                layout(item->screen);
            break;

        default:
            break;
    }

    return QObject::eventFilter( object, event );
}

class LayoutEvent : public QEvent
{
public:
    LayoutEvent(int screen)
    : QEvent(QEvent::User), _screen(screen)
    {
    }

    int screen() const 
    {
        return _screen;
    }

private:
    int _screen;
};

void ServerLayoutManager::customEvent(QEvent *e)
{
    if(e->type() == QEvent::User) {
        dolayout(static_cast<LayoutEvent *>(e)->screen());
    } 
}

void ServerLayoutManager::layout(int screen)
{
    QEvent *e = new LayoutEvent(screen);
    QCoreApplication::postEvent(this, e);
}

void ServerLayoutManager::dolayout(int screen)
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect mwr(desktop->screenGeometry(screen));
    qLog(UI) << "Layout screen docking" << screen;
    foreach (Item *item, docked[screen]) {
        QWidget *w = item->w;
        if ( !w->isVisible() )
            continue;
        QSize sh = w->sizeHint();
        QSize fs = item->fixed.isValid() ? item->fixed : sh;
        switch ( item->p ) {
            case WindowManagement::Top:
                w->setGeometry(mwr.left(), mwr.top(),
                    mwr.width(), fs.height() );
                mwr.setTop( mwr.top() + fs.height() );
                break;
            case WindowManagement::Bottom:
                w->setGeometry( mwr.left(), mwr.bottom()-fs.height()+1,
                    mwr.width(), fs.height() );
                mwr.setBottom( mwr.bottom()-fs.height() );
                break;
            case WindowManagement::Left:
                w->setGeometry( mwr.left(), mwr.top(),
                    fs.width(), mwr.height() );
                mwr.setLeft( w->geometry().right() + 1 );
                break;
            case WindowManagement::Right:
                w->setGeometry( mwr.right()-fs.width()+1, mwr.top(),
                    fs.width(), mwr.height() );
                mwr.setRight( w->geometry().left() - 1 );
                break;
        }
    }

#ifdef Q_WS_QWS
    qLog(UI) << " set max window rect for screen" << screen << mwr;
    QWSServer::setMaxWindowRect( mwr );
#endif
}

ServerLayoutManager::Item *ServerLayoutManager::findWidget( const QWidget *w ) const
{
    foreach (QList<Item*> screenItems, docked) {
        foreach (Item *item, screenItems) {
            if ( item->w == w )
                return item;
        }
    }

    return 0;
}
// declare WindowManagementPrivate
class WindowManagementPrivate : public QObject
{
    Q_OBJECT
public:
    WindowManagementPrivate(QObject * = 0);
    void protectWindow(QWidget *);

signals:
    void windowActive(const QString &, const QRect &, QWSWindow *);
    void windowCaption(const QString &);

private slots:
    void windowEvent(QWSWindow *, QWSServer::WindowEvent);
    void destroyed(QObject *);

private:
    void doWindowActive(const QString &, const QRect &, QWSWindow *);
    QValueSpaceObject wmValueSpace;
    QSet<QWidget *> widgets;
};
extern QWSServer *qwsServer;
Q_GLOBAL_STATIC(WindowManagementPrivate, windowManagement);

// define WindowManagementPrivate
WindowManagementPrivate::WindowManagementPrivate(QObject *parent)
: QObject(parent), wmValueSpace("/UI/ActiveWindow")
{
    Q_ASSERT(qwsServer);
    connect(qwsServer, SIGNAL(windowEvent(QWSWindow*,QWSServer::WindowEvent)),
            this, SLOT(windowEvent(QWSWindow*,QWSServer::WindowEvent)) );
}

void WindowManagementPrivate::protectWindow(QWidget *wid)
{
    if(wid->isVisible())
        wid->raise();

    if(!widgets.contains(wid)) {
        widgets.insert(wid);
        QObject::connect(wid, SIGNAL(destroyed(QObject *)),
                         this, SLOT(destroyed(QObject *)));
    }
}

void WindowManagementPrivate::windowEvent(QWSWindow *w,
                                          QWSServer::WindowEvent e)
{
    if (!w)
        return;
    static int active = 0;

    bool known = false;
    for(QSet<QWidget *>::ConstIterator iter = widgets.begin();
            !known && widgets.end() != iter;
            ++iter)
        known = ((*iter)->winId() == (unsigned)w->winId());

    switch( e ) {
        case QWSServer::Raise:
            if (!w->isVisible())
                break;
            // else FALL THROUGH
        case QWSServer::Show:
            {
                if(!known
                    && w->name() != "_fullscreen_") { // bogus - Qt needs to send window flags to server.
                    QRect req = w->requestedRegion().boundingRect();
                    QSize s(qt_screen->deviceWidth(),
                            qt_screen->deviceHeight());
                    req = qt_screen->mapFromDevice(req, s);

                    for(QSet<QWidget *>::ConstIterator iter = widgets.begin();
                            !known && widgets.end() != iter;
                            ++iter)
                        if((*iter)->isVisible() &&
                                req.intersects((*iter)->rect())) {
                            QTimer::singleShot(0, *iter, SLOT(raise()));
                        }
                }
            }
            // FALL THROUGH
        case QWSServer::Name:
        case QWSServer::Active:
            if (w->caption() == QLatin1String("_ignore_"))
                break;
            if (e == QWSServer::Active)
                active = w->winId();

            if(active == w->winId() && !known) {
                QRect req = w->requestedRegion().boundingRect();
                QSize s(qt_screen->deviceWidth(), qt_screen->deviceHeight());
                req = qt_screen->mapFromDevice(req, s);

                doWindowActive(w->caption(), req, w);
            }
            break;
        default:
            break;
    }
}

void WindowManagementPrivate::doWindowActive(const QString &caption,
                                             const QRect &rect, QWSWindow *win)
{
    wmValueSpace.setAttribute("Title", caption);
    wmValueSpace.setAttribute("Rect", rect);

    emit windowActive(caption, rect, win);
    if(rect.top() <= qt_screen->deviceHeight()/3 &&
       rect.bottom() > qt_screen->deviceHeight()/2 &&
       rect.width() >= qt_screen->deviceWidth()-4) {
        wmValueSpace.setAttribute("Caption", caption);
        emit windowCaption(caption);
    }

}

void WindowManagementPrivate::destroyed(QObject *obj)
{
    QWidget * wid = static_cast<QWidget *>(obj);
    widgets.remove(wid);
}

// define WindowManagement
/*!
  \class WindowManagement
  \ingroup QtopiaServer
  \brief The WindowManagement class allows you to monitor and control the
         application windows in the system.

  The WindowManagement class allows server windows to be protected, preventing
  other windows from obscuring them, and docked to the edge of the screen.

  The WindowManagement class also updates the value space with the following
  keys that can be used to track window status:
  \table
  \header \o Key \o Description
  \row \o \c {/UI/ActiveWindow/Title} \o The title of the currently active window
  \row \o \c {/UI/ActiveWindow/Rect} \o The rectangle of the currently active window
  \row \o \c {/UI/ActiveWindow/Caption} \o The caption of the currently active window.  The caption is the same as the title except that it isn't changed for popup windows that only consume a small part of the screen.
  \endtable

  These value space keys are updated if either the \c {WindowManagement} task is
  running, or at least one instance of the WindowManagement class has been
  instantiated.
 */

/*!
  \fn void WindowManagement::windowActive(const QString &caption, const QRect &rect, QWSWindow *window)

  Emitted whenever a \a window becomes active or the active window's \a caption
  changes.  \a rect is the rectangle covered by the window.
 */

/*!
  \enum WindowManagement::DockArea

  The DockArea enum type defines the areas where widgets can be docked:
  \value Top - the top of the screen.
  \value Bottom - the Bottom of the screen.
  \value Left - the Left of the screen.
  \value Right - the Right of the screen.
*/

/*!
  Construct a new WindowManagement instance with the specified \a parent.
 */
WindowManagement::WindowManagement(QObject *parent)
: QObject(parent), d(windowManagement())
{
    if(d) {
        QObject::connect(d, SIGNAL(windowActive(const QString &, const QRect &, QWSWindow *)),
                this, SIGNAL(windowActive(const QString &, const QRect &, QWSWindow *)));
        QObject::connect(d, SIGNAL(windowCaption(const QString &)),
                this, SIGNAL(windowCaption(const QString &)));
    }
}

/*!
  \fn void WindowManagement::windowCaption(const QString &caption)

  Emitted whenever the active window \a caption changes.
  */

/*!
  \internal
  */
WindowManagement::~WindowManagement()
{
}

/*!
  Prevent applications windows from being raised above the provided toplevel
  \a window.  If an application window attempts to obscure any of the protected
  region, the protected window will immediately be raised above it.
 */
void WindowManagement::protectWindow(QWidget *window)
{
    Q_ASSERT(window->isWindow());
    WindowManagementPrivate *d = windowManagement();
    if(d)
        d->protectWindow(window);
}

/*!
  Docks a top-level widget \a window on a side of the \a screen specified by
  \a placement.  The widget is placed according to the order that it was
  docked, its sizeHint() and whether previously docked widgets are visible.
  The desktop area available to QWidget::showMaximized() will exclude any
  visible docked widgets.

  For example, if a widget is docked at the bottom of the screen, its sizeHint()
  will define its height and it will use the full width of the screen.  If a
  widget is then docked to the right, its sizeHint() will define its width and
  it will be as high as possible without covering the widget docked at the
  bottom.

  This function is useful for reserving system areas such as taskbars
  and input methods that should not be covered by applications.  Even after
  calling this method, an application can manually position itself over the
  docked window.  To prevent this, also call WindowManagement::protectWindow()
  on the window.
*/
void WindowManagement::dockWindow(QWidget *window, DockArea placement, int screen)
{
    dockWindow(window, placement, QSize(), screen);
}

/*!
  \overload

  Normally the QWidget::sizeHint() of the docked widget is used to determine
  its docked size.  If the sizeHint() is not correct, the \a size parameter can
  be used to override it.  The \a window, \a placement and \a screen parameters
  are used as above.
 */
void WindowManagement::dockWindow(QWidget *window, DockArea placement,
                                  const QSize &size, int screen)
{
    Q_ASSERT(window->isWindow());

    static ServerLayoutManager *lm = 0;

    if ( !lm )
        lm = new ServerLayoutManager;

    lm->addDocked(window, placement, size, screen);
}

QTOPIA_STATIC_TASK(WindowManagement, windowManagement());
#include "windowmanagement.moc"
