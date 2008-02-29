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

#include "qlabel.h"
#include "qx11info_x11.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qbitmap.h"
#include "qevent.h"
#include "qapplication.h"
#include "qlist.h"
#include "qmenu.h"
#include "qtimer.h"
#include "qsystemtrayicon_p.h"

Window QSystemTrayIconSys::sysTrayWindow = None;
QList<QSystemTrayIconSys *> QSystemTrayIconSys::trayIcons;
QCoreApplication::EventFilter QSystemTrayIconSys::oldEventFilter = 0;
Atom QSystemTrayIconSys::sysTraySelection = None;

// Locate the system tray
Window QSystemTrayIconSys::locateSystemTray()
{
    Display *display = QX11Info::display();
    if (sysTraySelection == None) {
        int screen = QX11Info::appScreen();
        QString net_sys_tray = QString::fromLatin1("_NET_SYSTEM_TRAY_S%1").arg(screen);
        sysTraySelection = XInternAtom(display, net_sys_tray.toLatin1(), False);
    }

    return XGetSelectionOwner(QX11Info::display(), sysTraySelection);
}

bool QSystemTrayIconSys::sysTrayTracker(void *message, long *result)
{
    bool retval = false;
    if (QSystemTrayIconSys::oldEventFilter)
        retval = QSystemTrayIconSys::oldEventFilter(message, result);

    if (trayIcons.isEmpty())
        return retval;

    Display *display = QX11Info::display();
    XAnyEvent *ev = (XAnyEvent *)message;
    if  (ev->type == DestroyNotify && ev->window == sysTrayWindow) {
	sysTrayWindow = locateSystemTray();
        for (int i = 0; i < trayIcons.count(); i++) {
            if (sysTrayWindow == None) {
	        QBalloonTip::hideBalloon();
                trayIcons[i]->hide(); // still no luck
	    } else
                trayIcons[i]->addToTray(); // add it to the new tray
        }
        retval = true;
    } else if (ev->type == ClientMessage && sysTrayWindow == None) {
        static Atom manager_atom = XInternAtom(display, "MANAGER", False);
        XClientMessageEvent *cm = (XClientMessageEvent *)message;
        if ((cm->message_type == manager_atom) && ((Atom)cm->data.l[1] == sysTraySelection)) {
	    sysTrayWindow = cm->data.l[2];
	    XSelectInput(display, sysTrayWindow, StructureNotifyMask);
            for (int i = 0; i < trayIcons.count(); i++) {
                trayIcons[i]->addToTray();
            }
            retval = true;
        }
    }

    return retval;
}

QSystemTrayIconSys::QSystemTrayIconSys(QSystemTrayIcon *q)
    : QWidget(0, Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint),
      q(q)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    static bool eventFilterAdded = false;
    Display *display = QX11Info::display();
    if (!eventFilterAdded) {
        oldEventFilter = qApp->setEventFilter(sysTrayTracker);
	eventFilterAdded = true;
	Window root = QX11Info::appRootWindow();
        XWindowAttributes attr;
        XGetWindowAttributes(display, root, &attr);
        if ((attr.your_event_mask & StructureNotifyMask) != StructureNotifyMask)
            XSelectInput(display, root, attr.your_event_mask | StructureNotifyMask); // for MANAGER selection
    }
    if (trayIcons.isEmpty()) {
        sysTrayWindow = locateSystemTray();
	if (sysTrayWindow != None)
	    XSelectInput(display, sysTrayWindow, StructureNotifyMask); // track tray events
    }
    trayIcons.append(this);
    setMouseTracking(true);
#ifndef QT_NO_TOOLTIP
    setToolTip(q->toolTip());
#endif
    if (sysTrayWindow != None)
        addToTray();
}

QSystemTrayIconSys::~QSystemTrayIconSys()
{
    trayIcons.removeAt(trayIcons.indexOf(this));
    if (trayIcons.isEmpty()) {
        Display *display = QX11Info::display();
        if (sysTrayWindow == None)
            return;
        XSelectInput(display, sysTrayWindow, 0); // stop tracking the tray
        sysTrayWindow = None;
    }
}

void QSystemTrayIconSys::addToTray()
{
    Q_ASSERT(sysTrayWindow != None);
    Display *display = QX11Info::display();
    Window wid = winId();

    // GNOME, NET WM Specification
    static Atom netwm_tray_atom = XInternAtom(display, "_NET_SYSTEM_TRAY_OPCODE", False);
    long l[5] = { CurrentTime, SYSTEM_TRAY_REQUEST_DOCK, wid, 0, 0 };
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = sysTrayWindow;
    ev.xclient.message_type = netwm_tray_atom;
    ev.xclient.format = 32;
    memcpy((char *)&ev.xclient.data, (const char *) l, sizeof(l));
    XSendEvent(display, sysTrayWindow, False, 0, &ev);
    setMinimumSize(22, 22); // required at least on GNOME
}

void QSystemTrayIconSys::updateIcon()
{
    cachedPixmap = q->icon().pixmap(size(), QIcon::Normal);
    if (!cachedPixmap.mask().isNull()) {
        QBitmap mask(size());
        mask.fill(Qt::color0);
        QBitmap pixMask = cachedPixmap.mask();
        QPainter p(&mask);
        p.drawPixmap((mask.width() - pixMask.width())/2, (mask.height() - pixMask.height())/2,
                     pixMask);
        setMask(mask);
    } else
	setMask(QBitmap());
    update();
}

void QSystemTrayIconSys::resizeEvent(QResizeEvent *re)
{
     QWidget::resizeEvent(re);
     updateIcon();
}

void QSystemTrayIconSys::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    const QRect r = rect();
    p.drawPixmap(r.x() + (r.width() - cachedPixmap.width())/2,
                 r.y() + (r.height() - cachedPixmap.height())/2,
                 cachedPixmap);
}

void QSystemTrayIconSys::mousePressEvent(QMouseEvent *ev)
{
    QPoint globalPos = ev->globalPos();
    if (ev->button() == Qt::RightButton && q->contextMenu())
        q->contextMenu()->popup(globalPos);

    if (ev->button() == Qt::LeftButton)
        emit q->activated(QSystemTrayIcon::Trigger);
    else if (ev->button() == Qt::RightButton)
        emit q->activated(QSystemTrayIcon::Context);
    else if (ev->button() == Qt::MidButton)
        emit q->activated(QSystemTrayIcon::MiddleClick);
}

void QSystemTrayIconSys::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        emit q->activated(QSystemTrayIcon::DoubleClick);
}

bool QSystemTrayIconSys::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        return QApplication::sendEvent(q, e);
    }
    return QWidget::event(e);
}

bool QSystemTrayIconSys::x11Event(XEvent *event)
{
    if (event->type == ReparentNotify)
        show();
    return QWidget::x11Event(event);
}

////////////////////////////////////////////////////////////////////////////
void QSystemTrayIconPrivate::install_sys()
{
    Q_Q(QSystemTrayIcon);
    if (!sys)
        sys = new QSystemTrayIconSys(q);
}

QPoint QSystemTrayIconPrivate::globalPos_sys() const
{
    return QPoint();
}

void QSystemTrayIconPrivate::remove_sys()
{
    if (!sys)
        return;
    QBalloonTip::hideBalloon();
    sys->hide(); // this should do the trick, but...
    delete sys; // wm may resize system tray only for DestroyEvents
    sys = 0;
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
    if (!sys)
        return;
    sys->updateIcon();
}

void QSystemTrayIconPrivate::updateMenu_sys()
{

}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
    if (!sys)
        return;
#ifndef QT_NO_TOOLTIP
    sys->setToolTip(toolTip);
#endif
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
    return QSystemTrayIconSys::locateSystemTray() != None;
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &message, const QString &title,
                                   QSystemTrayIcon::MessageIcon icon, int msecs)
{
    if (!sys)
        return;
    QPoint g = sys->mapToGlobal(QPoint(0, 0));
    QBalloonTip::showBalloon(icon, message, title, sys->q,
                             QPoint(g.x() + sys->width()/2, g.y() + sys->height()/2),
                             msecs);
}
