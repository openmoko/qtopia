/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qwidget.h>
#include <qdrag.h>
#include <qpixmap.h>
#include <qpoint.h>
#include "qdnd_p.h"

#ifndef QT_NO_DRAGANDDROP

/*!
    \class QDrag
    \brief The QDrag class provides support for MIME-based drag and drop data
    transfer.

    Drag and drop is an intuitive way for users to copy or move data around in an
    application, and is used in many desktop environments as a mechanism for copying
    data between applications. Drag and drop support in Qt is centered around the
    QDrag class that handles most of the details of a drag and drop operation.

    The data to be transferred by the drag and drop operation is contained in a
    QMimeData object. This is specified with the setMimeData() function in the
    following way:

    \quotefromfile snippets/dragging/mainwindow.cpp
    \skipto mousePressEvent
    \skipto QDrag
    \printuntil setMimeData

    Note that setMimeData() assigns ownership of the QMimeData object to the
    QDrag object. The QDrag must be constructed on the heap with a parent QObject
    to ensure that Qt can clean up after the drag and drop operation has been
    completed.

    A pixmap can be used to represent the data while the drag is in progress, and
    will move with the cursor to the drop target. This pixmap typically shows an
    icon that represents the MIME type of the data being transferred, but any
    pixmap can be set with setPixmap(). Care must be taken to ensure that the
    pixmap is not too large. The cursor's hot spot can be given a position relative
    to the top-left corner of the pixmap with the setHotSpot() function. The
    following code positions the pixmap so that the cursor's hot spot points to
    the center of its bottom edge:

    \quotefromfile snippets/separations/finalwidget.cpp
    \skipto setHotSpot
    \printuntil ;

    \bold{Note:} on X11, the pixmap may not be able to keep up with the mouse
    movements if the hot spot causes the pixmap to be displayed
    directly under the cursor.

    The source and target widgets can be found with source() and target().
    These functions are often used to determine whether drag and drop operations
    started and finished at the same widget, so that special behavior can be
    implemented.

    QDrag only deals with the drag and drop operation itself. It is up to the
    developer to decide when a drag operation begins, and how a QDrag object should
    be constructed and used. For a given widget, it is often necessary to
    reimplement \l{QWidget::mousePressEvent()}{mousePressEvent()} to determine
    whether the user has pressed a mouse button, and reimplement
    \l{QWidget::mouseMoveEvent()}{mouseMoveEvent()} to check whether a QDrag is
    required.

    \sa {Drag and Drop}, QClipboard, QMimeData, QWindowsMime, QMacPasteboardMime,
        {Draggable Icons Example}, {Draggable Text Example}, {Drop Site Example},
	    {Fridge Magnets Example}
*/

/*!
    Constructs a new drag object for the widget specified by \a dragSource.
*/
QDrag::QDrag(QWidget *dragSource)
    : QObject(*new QDragPrivate, dragSource)
{
    Q_D(QDrag);
    d->source = dragSource;
    d->target = 0;
    d->data = 0;
    d->hotspot = QPoint(-10, -10);
    d->possible_actions = Qt::CopyAction;
    d->executed_action = Qt::IgnoreAction;
    d->defaultDropAction = Qt::IgnoreAction;
}

/*!
    Destroys the drag object.
*/
QDrag::~QDrag()
{
    Q_D(QDrag);
    delete d->data;
    QDragManager *manager = QDragManager::self();
    if (manager && manager->object == this)
        manager->cancel(false);
}

/*!
    Sets the data to be sent to the given MIME \a data. Ownership of the data is
    transferred to the QDrag object.
*/
void QDrag::setMimeData(QMimeData *data)
{
    Q_D(QDrag);
    if (d->data == data)
        return;
    if (d->data != 0)
        delete d->data;
    d->data = data;
}

/*!
    Returns the MIME data that is encapsulated by the drag object.
*/
QMimeData *QDrag::mimeData() const
{
    Q_D(const QDrag);
    return d->data;
}

/*!
    Sets \a pixmap as the pixmap used to represent the data in a drag
    and drop operation. You can only set a pixmap before the drag is
    started.
*/
void QDrag::setPixmap(const QPixmap &pixmap)
{
    Q_D(QDrag);
    d->pixmap = pixmap;
}

/*!
    Returns the pixmap used to represent the data in a drag and drop operation.
*/
QPixmap QDrag::pixmap() const
{
    Q_D(const QDrag);
    return d->pixmap;
}

/*!
    Sets the position of the hot spot relative to the top-left corner of the
    pixmap used to the point specified by \a hotspot.

    \bold{Note:} on X11, the pixmap may not be able to keep up with the mouse
    movements if the hot spot causes the pixmap to be displayed
    directly under the cursor.
*/
void QDrag::setHotSpot(const QPoint& hotspot)
{
    Q_D(QDrag);
    d->hotspot = hotspot;
}

/*!
    Returns the position of the hot spot relative to the top-left corner of the
    cursor.
*/
QPoint QDrag::hotSpot() const
{
    Q_D(const QDrag);
    return d->hotspot;
}

/*!
    Returns the source of the drag object. This is the widget where the drag
    and drop operation originated.
*/
QWidget *QDrag::source() const
{
    Q_D(const QDrag);
    return d->source;
}

/*!
    Returns the target of the drag and drop operation. This is the widget where
    the drag object was dropped.
*/
QWidget *QDrag::target() const
{
    Q_D(const QDrag);
    return d->target;
}

/*!
    \since 4.3

    Starts the drag and drop operation and returns a value indicating the requested
    drop action when it is completed. The drop actions that the user can choose
    from are specified in \a supportedActions. The default proposed action will be selected
    among the allowed actions in the following order: Move, Copy and Link.

    \bold{Note:} On Linux and Mac OS X, the drag and drop operation
    can take some time, but this function does not block the event
    loop. Other events are still delivered to the application while
    the operation is performed. On Windows, the Qt event loop is
    blocked while during the operation.
*/

Qt::DropAction QDrag::exec(Qt::DropActions supportedActions)
{
    return exec(supportedActions, Qt::IgnoreAction);
}

/*!
    \since 4.3

    Starts the drag and drop operation and returns a value indicating the requested
    drop action when it is completed. The drop actions that the user can choose
    from are specified in \a supportedActions.

    The \a defaultDropAction determines which action will be proposed when the user performs a
    drag without using modifier keys.

    \bold{Note:} Although the drag and drop operation is blocking, the function
    does not block the event loop. Other events are still delivered to the application
    while the operation is performed.
*/

Qt::DropAction QDrag::exec(Qt::DropActions supportedActions, Qt::DropAction defaultDropAction)
{
    Q_D(QDrag);
    if (!d->data) {
        qWarning("QDrag: No mimedata set before starting the drag");
        return d->executed_action;
    }
    QDragManager *manager = QDragManager::self();
    d->defaultDropAction = Qt::IgnoreAction;
    d->possible_actions = supportedActions;

    if (manager) {
        if (defaultDropAction == Qt::IgnoreAction) {
            if (supportedActions & Qt::MoveAction) {
                d->defaultDropAction = Qt::MoveAction;
            } else if (supportedActions & Qt::CopyAction) {
                d->defaultDropAction = Qt::CopyAction;
            } else if (supportedActions & Qt::LinkAction) {
                d->defaultDropAction = Qt::LinkAction;
            }
        } else {
            d->defaultDropAction = defaultDropAction;
        }
        d->executed_action = manager->drag(this);
    }

    return d->executed_action;
}

/*!
    \obsolete

    \bold{Note:} It is recommended to use exec() instead of this function.

    Starts the drag and drop operation and returns a value indicating the requested
    drop action when it is completed. The drop actions that the user can choose
    from are specified in \a request. Qt::CopyAction is always allowed.

    \bold{Note:} Although the drag and drop operation can take some time, this function
    does not block the event loop. Other events are still delivered to the application
    while the operation is performed.

    \sa exec()
*/
Qt::DropAction QDrag::start(Qt::DropActions request)
{
    Q_D(QDrag);
    if (!d->data) {
        qWarning("QDrag: No mimedata set before starting the drag");
        return d->executed_action;
    }
    QDragManager *manager = QDragManager::self();
    d->defaultDropAction = Qt::IgnoreAction;
    d->possible_actions = request | Qt::CopyAction;
    if (manager)
        d->executed_action = manager->drag(this);
    return d->executed_action;
}

/*!
    Sets the drag \a cursor for the \a action. This allows you
    to override the default native cursors. To revert to using the
    native cursor for \a action pass in a null QPixmap as \a cursor.

    The \a action can only be CopyAction, MoveAction or LinkAction.
    All other values of DropAction are ignored.
*/
void QDrag::setDragCursor(const QPixmap &cursor, Qt::DropAction action)
{
    Q_D(QDrag);
    if (action != Qt::CopyAction && action != Qt::MoveAction && action != Qt::LinkAction)
        return;
    if (cursor.isNull())
        d->customCursors.remove(action);
    else
        d->customCursors[action] = cursor;
}

/*!
    \fn void QDrag::actionChanged(Qt::DropAction action)

    This signal is emitted when the \a action associated with the
    drag changes.

    \sa targetChanged()
*/

/*!
    \fn void QDrag::targetChanged(QWidget *newTarget)

    This signal is emitted when the target of the drag and drop
    operation changes, with \a newTarget the new target.

    \sa target(), actionChanged()
*/

#endif // QT_NO_DRAGANDDROP
