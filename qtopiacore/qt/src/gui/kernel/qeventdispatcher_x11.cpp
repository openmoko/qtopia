/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.1, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qeventdispatcher_x11_p.h"

#include "qapplication.h"
#include "qx11info_x11.h"

#include "qt_x11_p.h"
#include <private/qeventdispatcher_unix_p.h>

class QEventDispatcherX11Private : public QEventDispatcherUNIXPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherX11)
public:
    inline QEventDispatcherX11Private()
        : xfd(-1)
    { }
    int xfd;
    QList<XEvent> queuedUserInputEvents;
};

QEventDispatcherX11::QEventDispatcherX11(QObject *parent)
    : QEventDispatcherUNIX(*new QEventDispatcherX11Private, parent)
{ }

QEventDispatcherX11::~QEventDispatcherX11()
{ }

bool QEventDispatcherX11::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QEventDispatcherX11);

    d->interrupt = false;
    QApplication::sendPostedEvents();

    ulong marker = XNextRequest(X11->display);
    int nevents = 0;
    do {
        while (!d->interrupt) {
            XEvent event;
            if (!(flags & QEventLoop::ExcludeUserInputEvents)
                && !d->queuedUserInputEvents.isEmpty()) {
                // process a pending user input event
                event = d->queuedUserInputEvents.takeFirst();
            } else if (XEventsQueued(X11->display, QueuedAlready)) {
                // process events from the X server
                XNextEvent(X11->display, &event);

                if (flags & QEventLoop::ExcludeUserInputEvents) {
                    // queue user input events
                    switch (event.type) {
                    case ButtonPress:
                    case ButtonRelease:
                    case MotionNotify:
                    case XKeyPress:
                    case XKeyRelease:
                    case EnterNotify:
                    case LeaveNotify:
                        d->queuedUserInputEvents.append(event);
                        continue;

                    case ClientMessage:
                        // only keep the wm_take_focus and
                        // _qt_scrolldone protocols, queue all other
                        // client messages
                        if (event.xclient.format == 32) {
                            if (event.xclient.message_type == ATOM(WM_PROTOCOLS) ||
                                (Atom) event.xclient.data.l[0] == ATOM(WM_TAKE_FOCUS)) {
                                break;
                            } else if (event.xclient.message_type == ATOM(_QT_SCROLL_DONE)) {
                                break;
                            }
                        }
                        d->queuedUserInputEvents.append(event);
                        continue;

                    default:
                        break;
                    }
                }
            } else {
                // no event to process
                break;
            }

            // send through event filter
            if (filterEvent(&event))
                continue;

            nevents++;
            if (qApp->x11ProcessEvent(&event) == 1)
                return true;

            if (event.xany.serial >= marker) {
                if (XEventsQueued(X11->display, QueuedAfterFlush))
                    flags &= ~QEventLoop::WaitForMoreEvents;
                goto out;
            }
        }
    } while (!d->interrupt && XEventsQueued(X11->display, QueuedAfterFlush));

 out:
    if (!d->interrupt) {
        const uint exclude_all =
            QEventLoop::ExcludeSocketNotifiers | QEventLoop::X11ExcludeTimers | QEventLoop::WaitForMoreEvents;
        if (nevents > 0 && ((uint)flags & exclude_all) == exclude_all) {
            QApplication::sendPostedEvents();
            return nevents > 0;
        }
        // return true if we handled events, false otherwise
        return QEventDispatcherUNIX::processEvents(flags) ||  (nevents > 0);
    }
    return nevents > 0;
}

bool QEventDispatcherX11::hasPendingEvents()
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return (qGlobalPostedEventsCount() || XPending(X11->display));
}

void QEventDispatcherX11::flush()
{
    XFlush(X11->display);
}

void QEventDispatcherX11::startingUp()
{
    Q_D(QEventDispatcherX11);
    d->xfd = XConnectionNumber(X11->display);
}

void QEventDispatcherX11::closingDown()
{
    Q_D(QEventDispatcherX11);
    d->xfd = -1;
}

int QEventDispatcherX11::select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                                timeval *timeout)
{
    Q_D(QEventDispatcherX11);
    if (d->xfd > 0) {
        nfds = qMax(nfds - 1, d->xfd) + 1;
        FD_SET(d->xfd, readfds);
    }
    return QEventDispatcherUNIX::select(nfds, readfds, writefds, exceptfds, timeout);
}
